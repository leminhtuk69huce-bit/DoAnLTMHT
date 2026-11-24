# TÀI LIỆU GIẢI THÍCH MÃ NGUỒN ĐỒ ÁN LẬP TRÌNH HỆ THỐNG MẠNG
## ĐỀ TÀI 13: HỆ THỐNG FTP SERVER & CLIENT V5 - BULK DOWNLOAD (TẢI HÀNG LOẠT)
**Giảng viên hướng dẫn: Thầy Bùi Phong**  
**Trường Đại học Xây dựng Hà Nội (HUCE)**  
*Mã nguồn tuân thủ 100% Framework hướng đối tượng Socket & CLI của Thầy - Không dùng thư viện ngoài*

---

## I. TỔNG QUAN KIẾN TRÚC HỆ THỐNG & ĐỐI CHIẾU SƠ ĐỒ BẢNG CỦA THẦY

Mô hình hệ thống phân tách rõ ràng thành 2 phân hệ độc lập: **Phân hệ Client** và **Phân hệ Server**, hoàn toàn kế thừa và mở rộng từ bộ khung lớp mẫu của Thầy Bùi Phong:

```
                            [ THƯ VIỆN CỐT LÕI CỦA THẦY ]
                                         │
        ┌────────────────────────────────┴────────────────────────────────┐
        ▼                                                                 ▼
[ PHÂN HỆ CLIENT ]                                               [ PHÂN HỆ SERVER ]
• Socket (Gốc)                                                  • Socket (Gốc)
  └── TCPSocket                                                    ├── TCPSocket (Data / Slave)
• TcpClient (Kênh điều khiển)                                      └── TCPServerSocket (Master nghe cổng 21)
  └── FTPClient (Tầng nghiệp vụ FTP & Bulk Download)             • TCPServer (Quản lý kết nối & luồng)
• CmdLineInterface (CLI của Thầy)                                  └── FTPServer (Nghiệp vụ FTP Server)
  └── FTPClientCLI (Giao diện người dùng)                        • Session (Lớp trừu tượng phiên)
                                                                   └── FTPSession (Xử lý USER, PASS, PASV, LIST, RETR)
                                                                 • ServerConfig (Quản lý cấu hình & Account)
                                                                   └── FTPServerConfig (Thư mục gốc & tài khoản)
                                                                 • CmdLineInterface (CLI của Thầy)
                                                                   └── FTPServerCLI (Giao diện Quản trị viên)
```

---

## II. GIẢI THÍCH CHI TIẾT PHÂN HỆ SERVER

### 1. Lớp `FTPServer` (Kế thừa `TCPServer`)
- **Mục đích**: Là trung tâm điều khiển của Server, lắng nghe trên cổng 21 và quản lý các kết nối đa luồng từ nhiều Client đồng thời.
- **Cách hoạt động**:
  1. `FTPServer::FTPServer(port)`: Gọi khởi tạo `TCPServer(21)`, gọi `initCmd()`, và gán `conf = new FTPServerConfig()`.
  2. `initCmd()`: Đăng ký các lệnh giao thức FTP với bảng lệnh của Server bằng macro `FUNC_CAST` của Thầy:
     - `USER` -> `&FTPSession::doUser`
     - `PASS` -> `&FTPSession::doPass`
     - `PASV` -> `&FTPSession::doPasv`
     - `LIST` -> `&FTPSession::doList`
     - `RETR` -> `&FTPSession::doRetr`
     - `QUIT` -> `&FTPSession::doQuit`
  3. `startNewSession(TcpSocket slave)`: Khi có Client kết nối tới cổng 21, Server tạo một luồng riêng (`std::thread`), chạy `startNewSession`, gửi lời chào `220 Welcome to FTP Server`, sau đó lặp nhận lệnh bằng `readCmd()`, phân tích tham số bằng `parseCmd()`, và điều phối đến `FTPSession` qua hàm `doCmd()`.

### 2. Lớp `FTPSession` (Kế thừa `Session`)
- **Mục đích**: Đại diện cho phiên làm việc tương tác với từng Client riêng biệt qua socket `slave`.
- **Cơ chế hoạt động**:
  - `doUser` & `doPass`: Lưu tên đăng nhập tạm thời, kiểm tra thông tin với `conf->authenticate(user, pass)`. Đúng mật khẩu trả về `230 User logged in, proceed`, sai trả về `530 Not logged in`.
  - `doPasv`: Mở một socket phụ `dataServer = new TcpServerSocket()` lắng nghe trên một cổng động (trong dải 20000 - 25000), sau đó gửi phản hồi chuẩn FTP: `227 Entering Passive Mode (127,0,0,1,p1,p2)` với `port = p1 * 256 + p2`.
  - `doList`: Chấp nhận kết nối trên `dataSocket`, đọc danh mục thư mục được yêu cầu (mặc định là `server_files/`), gửi danh sách qua kênh dữ liệu, đóng kênh dữ liệu rồi gửi `226 Directory send OK`.
  - `doRetr`: Đọc tệp tin nhị phân từ thư mục gốc `server_files/`, gửi mã `150 Opening BINARY mode...`, bơm dữ liệu qua kênh `dataSocket` theo từng block 4KB, sau khi xong gửi `226 Transfer complete`.
  - `doQuit`: Gửi `221 Goodbye`, đặt `quitSession = true` để kết thúc vòng lặp phiên.

### 3. Lớp `FTPServerConfig` (Kế thừa `ServerConfig`)
- Quản lý thư mục gốc (`server_files`), cấu hình thời gian timeout và danh sách đối tượng `Account`.
- Hỗ trợ thêm tài khoản linh hoạt qua phương thức `addFTPAccount(user, pass)`.

### 4. Lớp `FTPServerCLI` (Kế thừa `CmdLineInterface`)
- Cung cấp console dòng lệnh riêng cho Quản trị viên điều khiển Server:
  - `start`: Khởi động Server.
  - `stop`: Dừng Server.
  - `status`: Kiểm tra trạng thái đang chạy, cổng, số tài khoản.
  - `adduser <user> <pass>`: Thêm hoặc đổi mật khẩu người dùng ngay lúc Server đang chạy.
  - `users`: Liệt kê tất cả tài khoản đã đăng ký.
  - `help`: Xem trợ giúp lệnh.

---

## III. GIẢI THÍCH CHI TIẾT PHÂN HỆ CLIENT

### 1. Lớp `FTPClient` (Kế thừa `TcpClient`)
- **Mục đích**: Thực thi các thủ tục giao thức FTP từ phía Client.
- **Cách thức hoạt động**:
  - `login(host, port, user, pass)`: Mở socket điều khiển `localsocket` kết nối tới Server, đọc mã chào mừng `220`, gửi `USER`, gửi `PASS`. Nếu mã nhận được là `230` thì đánh dấu xác thực thành công (`isAuthed = true`).
  - `openDataConnection(dataSocket)`: Gửi lệnh `PASV`, phân tích chuỗi phản hồi `(h1,h2,h3,h4,p1,p2)` để lấy IP và Port kênh dữ liệu, sau đó kết nối `dataSocket` tới đó.
  - `listFiles(folder)`: Mở kênh dữ liệu PASV, gửi `LIST <folder>`, nhận toàn bộ danh sách tệp tin dạng chuỗi văn bản và bóc tách thành vector danh sách tên file.
  - `downloadFile(remotePath)`: Mở kênh dữ liệu PASV, gửi `RETR <remotePath>`, mở tệp tin nhị phân tại `client_downloads/<tên_file>`, đọc từng khối dữ liệu từ kênh data cho đến khi Server đóng kết nối, sau đó kiểm tra mã kết thúc `226`.

### 2. Thuật toán `bulkDownload(folder)` (Trọng tâm Đề tài 13)
Đây là tính năng cốt lõi của đề tài "Tải hàng loạt thư mục":
```cpp
void FTPClient::bulkDownload(const string& folder)
{
    // Bước 1: Liệt kê toàn bộ các mục trong thư mục trên Server
    vector<string> items = listFiles(folder);
    
    // Bước 2: Lọc ra các tập tin thực sự (loại bỏ thư mục con kết thúc bằng '/')
    vector<string> filesToDownload;
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i].back() != '/') filesToDownload.push_back(items[i]);
    }
    
    // Bước 3: Lần lượt tải từng tệp tin và báo cáo tiến độ trực quan
    int successCount = 0;
    for (int i = 0; i < filesToDownload.size(); ++i) {
        cout << "[" << (i+1) << "/" << filesToDownload.size() << "] Dang tai: " << filesToDownload[i] << "..." << endl;
        if (downloadFile(remotePath)) successCount++;
    }
    
    // Bước 4: Tổng kết số file tải thành công
    cout << "Ket qua: " << successCount << "/" << filesToDownload.size() << " file tai ve thanh cong." << endl;
}
```

### 3. Lớp `FTPClientCLI` (Kế thừa `CmdLineInterface`)
- Đăng ký các lệnh thông qua `CLI_CAST`:
  - `login <ip> <user> <pass> [port]`: Đăng nhập vào FTP Server.
  - `list [folder]`: Xem danh sách file trong thư mục.
  - `get <file>`: Tải 1 file đơn lẻ.
  - `mget [folder]`: Tải toàn bộ thư mục về máy (Bulk Download).
  - `help`: Hướng dẫn cú pháp.
  - `quit`: Thoát Client.

---

## IV. BỘ CÂU HỎI THƯỜNG GẶP CỦA THẦY VÀ CÁCH TRẢ LỜI

**Câu 1: Dự án của em sử dụng những thư viện nào? Có dùng thư viện ngoài không?**
- *Trả lời*: Dạ thưa Thầy, dự án của em **100% sử dụng thư viện Socket và CLI do Thầy cung cấp** trong môn học (`Socket`, `TcpSocket`, `TcpServerSocket`, `TcpClient`, `TCPServer`, `Session`, `ServerConfig`, `CmdLineInterface`). Em chỉ liên kết với thư viện hệ điều hành Windows chuẩn `ws2_32` để Winsock hoạt động, hoàn toàn không dùng bất kỳ thư viện bên ngoài hay mã lạ nào.

**Câu 2: Cơ chế truyền dữ liệu trong FTP khác gì so với HTTP hoặc Echo thông thường?**
- *Trả lời*: Dạ thưa Thầy, FTP sử dụng **2 kênh kết nối TCP độc lập**:
  1. **Kênh điều khiển (Control Connection)**: Mở trên cổng 21, duy trì xuyên suốt phiên để gửi lệnh (`USER`, `PASS`, `LIST`, `RETR`, `QUIT`) và nhận mã trạng thái.
  2. **Kênh dữ liệu (Data Connection)**: Được mở động theo chế độ bị động (`PASV`). Khi cần truyền danh sách thư mục hoặc nội dung file, Server cấp 1 cổng tạm thời, Client kết nối vào cổng đó để truyền nhị phân. Khi truyền xong thì kênh này được đóng lại ngay, không ảnh hưởng đến kênh điều khiển.

**Câu 3: Đề tài của em là Bulk Download (Đề tài 13), em giải quyết như thế nào trong mã nguồn?**
- *Trả lời*: Dạ thưa Thầy, tính năng được cài đặt trong hàm `bulkDownload()` của lớp `FTPClient`:
  - Client gửi lệnh `LIST` để Server trả về danh sách toàn bộ các file trong thư mục chỉ định.
  - Client lọc ra các file hợp lệ, hiển thị số lượng file cần tải.
  - Sau đó Client dùng vòng lặp tự động gửi lệnh `RETR` tải tuần tự từng file về thư mục `client_downloads/` của máy Client, kèm theo hiển thị chỉ số tiến trình `[i/N]`.

**Câu 4: Server của em xử lý đồng thời nhiều Client ra sao?**
- *Trả lời*: Dạ thưa Thầy, lớp `TCPServer` của Thầy cài đặt cơ chế đa luồng: Khi có kết nối mới qua `master.accept()`, Server tạo một `std::thread` chạy phương thức `startNewSession()` và gọi `t.detach()`. Do đó mỗi Client sẽ có một phiên `FTPSession` độc lập chạy trên một luồng riêng, không làm nghẽn Server.
