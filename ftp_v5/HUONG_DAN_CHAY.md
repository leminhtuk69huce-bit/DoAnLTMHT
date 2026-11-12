# Hướng Dẫn Biên Dịch & Chạy (Bản Tối Giản: Login & Truyền File)

## 1. Cấu trúc mã nguồn
```
ftp_v5/
├── FTPServer.h          (Khai báo thư viện & hàm Server)
├── FTPServer.cpp        (Triển khai logic Server)
├── ServerMain.cpp       (Hàm main Server)
├── FTPClient.h          (Khai báo thư viện & hàm Client)
├── FTPClient.cpp        (Triển khai logic Client)
├── ClientMain.cpp       (Hàm main Client)
├── server_files/        (Thư mục chứa file của Server)
└── client_downloads/    (Thư mục lưu file Client tải về)
```

## 2. Biên dịch trên Code::Blocks hoặc g++
- Other linker options: `-lws2_32 -static`

## 3. Các lệnh sử dụng
### Server:
- `start`: Khởi động Server trên cổng 21 (chạy quyền Admin).
- `stop`: Tắt Server.
- `help`: Xem hướng dẫn.
- `exit`: Thoát chương trình.

### Client:
- `login <IP> <user> <pass>`: Đăng nhập (Mặc định: `login 127.0.0.1 admin 1234` hoặc `login 127.0.0.1 user1 1234`).
- `get <tên_file>`: Tải/truyền file từ Server về máy (Ví dụ: `get docs/file1.txt` hoặc `get file1.txt`). File tải về sẽ được lưu tại `client_downloads/`.
- `help`: Xem hướng dẫn.
- `quit`: Thoát và đóng kết nối.
