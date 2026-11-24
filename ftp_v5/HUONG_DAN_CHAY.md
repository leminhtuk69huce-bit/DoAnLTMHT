# HƯỚNG DẪN CHẠY HỆ THỐNG FTP SERVER & CLIENT V5 (BULK DOWNLOAD)
**Đề tài 13 - Lập trình hệ thống mạng (HUCE)**
*Xây dựng theo Framework hướng đối tượng của Thầy Bùi Phong*

---

## 1. HƯỚNG DẪN CHẠY NHANH (2 CỬA SỔ TERMINAL)

Để chạy thử nghiệm, bạn mở **2 cửa sổ Terminal** (Command Prompt hoặc PowerShell):

### BƯỚC 1: KHỞI ĐỘNG SERVER (Cửa sổ 1)
1. Mở Terminal và di chuyển vào thư mục dự án:
   ```powershell
   cd E:\LTHTM\ftp_v5
   ```
2. Chạy Server:
   ```powershell
   .\FTPServer.exe
   ```
3. Server sẽ tự động mở cổng 21 và hiển thị:
   ```
   [*] FTP Server da tu dong khoi dong tren cong 21!
   [*] San sang tiep nhan ket noi tu FTP Client.
   ftp_server>
   ```

*Các lệnh bạn có thể thử tại Server:*
* `status` : Xem trạng thái hoạt động của Server.
* `users`  : Xem danh sách các tài khoản người dùng đã có.
* `adduser <user> <pass>` : Thêm người dùng mới (ví dụ: `adduser ha 123456`).
* `help`   : Xem bảng hướng dẫn lệnh.

---

### BƯỚC 2: KHỞI ĐỘNG VÀ ĐIỀU KHIỂN CLIENT (Cửa sổ 2)
1. Mở cửa sổ Terminal thứ hai và chuyển vào thư mục:
   ```powershell
   cd E:\LTHTM\ftp_v5
   ```
2. Chạy Client:
   ```powershell
   .\FTPClient.exe
   ```
3. Đăng nhập vào Server:
   ```
   login 127.0.0.1 admin admin123
   ```
   *(Hoặc tài khoản sinh viên: `login 127.0.0.1 student huce123`)*

4. **Xem danh sách file trên Server:**
   ```
   list
   ```
   Hoặc xem thư mục con `docs`:
   ```
   list docs
   ```

5. **Tải 1 file đơn lẻ (Lệnh `get`):**
   ```
   get readme.txt
   ```
   *(File tải về được lưu tại thư mục: `client_downloads/`)*

6. **TẢI TOÀN BỘ THƯ MỤC - BULK DOWNLOAD (Lệnh `mget` - Trọng tâm Đề tài 13):**
   ```
   mget docs
   ```
   Client sẽ tự động quét toàn bộ thư mục `docs`, lần lượt tải tất cả các file (`file1.txt`, `file2.txt`, `file3.txt`) và hiển thị thanh tiến độ `[1/3]`, `[2/3]`, `[3/3]`.

7. **Thoát chương trình:**
   ```
   quit
   ```

---

## 2. NƠI LƯU TRỮ DỮ LIỆU
* **Dữ liệu trên Server:** nằm trong thư mục `./server_files/` (chứa các file mẫu và thư mục con `docs/`).
* **Dữ liệu tải về của Client:** được lưu tự động trong thư mục `./client_downloads/`.

---

## 3. LỆNH BIÊN DỊCH LẠI TỪ MÃ NGUỒN (NẾU SỬA CODE)

Nếu bạn có chỉnh sửa mã nguồn C++, mở Terminal tại thư mục `ftp_v5` và chạy 2 lệnh sau:

* **Biên dịch Server:**
  ```powershell
  g++ -std=c++14 ServerMain.cpp FTPServer.cpp ftpservercli.cpp ftpserverconfig.cpp ftpsession.cpp cli.cpp socket.cpp tcpsocket.cpp tcpserversocket.cpp tcpserver.cpp session.cpp serverconfig.cpp -lws2_32 -o FTPServer.exe
  ```

* **Biên dịch Client:**
  ```powershell
  g++ -std=c++14 ClientMain.cpp FTPClient.cpp ftpclientcli.cpp cli.cpp socket.cpp tcpsocket.cpp tcpclient.cpp -lws2_32 -o FTPClient.exe
  ```
