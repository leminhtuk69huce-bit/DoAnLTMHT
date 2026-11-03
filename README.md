# Đồ Án Lập Trình Hệ Thống Mạng: FTP v5 (Bulk Download)

Chương trình Client - Server viết bằng **C++ Winsock** mô phỏng giao thức FTP phiên bản 5 cho phép tải về toàn bộ các tệp tin trong một thư mục chỉ định từ máy chủ về máy khách (Bulk Download).

---

## 1. Yêu Cầu Đề Tài & Các Lệnh Hoạt Động

### Phía Server:
- `start`: Khởi động máy chủ, tự động hiển thị IP và lắng nghe kết nối tại cổng điều khiển `21`.
- `stop`: Tạm dừng máy chủ.
- `ip`: Hiển thị lại các địa chỉ IP của server (Localhost & LAN).
- `help`: Hiển thị danh sách hướng dẫn lệnh.
- `exit`: Tắt máy chủ và thoát chương trình.

### Phía Client:
- `login <IP server> <username> <password>`: Kết nối và xác thực theo giao thức FTP (Tài khoản mặc định: `admin` / `1234`).
- `mget <tên thư mục server>`: Client gửi yêu cầu lấy danh sách tệp. Với mỗi tệp, client tự động tạo kết nối dữ liệu (Data Connection), tải tệp về thư mục `client_downloads/` và hiển thị tiến trình (VD: `Đã tải 1/3 file: file1.txt`).
- `help`: Hiển thị danh sách hướng dẫn lệnh.
- `quit`: Đóng phiên làm việc và ngắt kết nối.

---

## 2. Cấu Trúc Thư Mục Dự Án (Chuẩn C++ .cpp & .h)

```
├── .gitignore               # Loại bỏ các file rác, file nhị phân, cấu hình cá nhân
├── README.md                # Tài liệu tổng quan dự án
├── LICENSE                  # Giấy phép mã nguồn
├── docs/                    # Tài liệu đặc tả và thiết kế hệ thống
│   ├── bao-cao-thiet-ke-ftp-v5.md  # Báo cáo phân tích thiết kế đầy đủ (UseCase, Sequence, Class...)
│   ├── de-tai-13-ftp-v5-dac-ta.md  # Đặc tả kỹ thuật chi tiết
│   └── tong-hop-de-tai-13.md       # Hướng dẫn tổng hợp & prompts
└── ftp_v5/
    ├── FTPServer.h          # File Header khai báo hàm và cấu hình Server
    ├── FTPServer.cpp        # File cài đặt chi tiết chức năng Server
    ├── ServerMain.cpp       # File chứa hàm main() điều khiển Server
    ├── FTPClient.h          # File Header khai báo hàm và cấu hình Client
    ├── FTPClient.cpp        # File cài đặt chi tiết chức năng Client
    ├── ClientMain.cpp       # File chứa hàm main() điều khiển Client
    ├── HUONG_DAN_CHAY.md    # Hướng dẫn chi tiết biên dịch trên Code::Blocks
    ├── server_files/        # Thư mục chứa dữ liệu trên Server
    │   └── docs/            # Thư mục mẫu chứa các file thử nghiệm (file1.txt, file2.txt,...)
    └── client_downloads/    # Thư mục lưu trữ các file sau khi Client tải về thành công
```

---

## 3. Hướng Dẫn Biên Dịch & Chạy Trên Code::Blocks

### A. Tạo Project Server:
1. Tạo project C++ Console trong Code::Blocks tên `FTPServer`.
2. Thêm 3 file: `FTPServer.h`, `FTPServer.cpp`, `ServerMain.cpp`.
3. Vào **Project -> Build Options -> Linker settings -> Other linker options**, thêm:
   ```text
   -lws2_32 -static
   ```
4. Build và chạy bằng quyền **Administrator** (để mở cổng 21).

### B. Tạo Project Client:
1. Tạo project C++ Console tên `FTPClient`.
2. Thêm 3 file: `FTPClient.h`, `FTPClient.cpp`, `ClientMain.cpp`.
3. Thêm linker options tương tự: `-lws2_32 -static`.
4. Build và chạy.

Xem hướng dẫn chi tiết từng bước tại [`ftp_v5/HUONG_DAN_CHAY.md`](ftp_v5/HUONG_DAN_CHAY.md).
