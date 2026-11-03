# Đồ Án Lập Trình Hệ Thống Mạng: FTP v5 (Bulk Download)

Chương trình Client - Server mô phỏng giao thức FTP phiên bản 5 cho phép tải về toàn bộ các tệp tin trong một thư mục chỉ định từ máy chủ về máy khách (Bulk Download).

---

## 1. Yêu Cầu Đề Tài & Các Lệnh Hoạt Động

### Phía Server:
- `start`: Khởi động máy chủ, lắng nghe kết nối tại cổng điều khiển `21`.
- `stop`: Tắt máy chủ và ngắt các kết nối.

### Phía Client:
- `login <IP server> <username> <password>`: Kết nối và xác thực theo giao thức FTP (Tài khoản mặc định: `admin` / `1234`).
- `mget <tên thư mục server>`: Client gửi yêu cầu lấy danh sách tệp. Với mỗi tệp, client tự động tạo kết nối dữ liệu (Data Connection), tải tệp về thư mục `client_downloads/` và hiển thị tiến trình (VD: `Đã tải 1/3 file...`).
- `quit`: Đóng phiên làm việc và ngắt kết nối.

---

## 2. Cấu Trúc Thư Mục Dự Án

```
├── .gitignore               # Loại bỏ các file rác, file nhị phân, cấu hình cá nhân
├── README.md                # Tài liệu tổng quan dự án
├── LICENSE                  # Giấy phép mã nguồn
└── ftp_v5/
    ├── FTPServer.cpp        # Mã nguồn C++ Winsock Server
    ├── FTPClient.cpp        # Mã nguồn C++ Winsock Client
    ├── server.py            # Mã nguồn Python Server (tham chiếu/chạy thử nghiệm)
    ├── client.py            # Mã nguồn Python Client (tham chiếu/chạy thử nghiệm)
    ├── HUONG_DAN_CHAY.md    # Hướng dẫn chi tiết biên dịch trên Code::Blocks
    ├── server_files/        # Thư mục chứa dữ liệu trên Server
    │   └── docs/            # Thư mục mẫu chứa các file thử nghiệm (file1.txt, file2.txt,...)
    └── client_downloads/    # Thư mục lưu trữ các file sau khi Client tải về thành công
```

---

## 3. Hướng Dẫn Biên Dịch & Chạy (C++ Winsock)

### Biên dịch với Code::Blocks / MinGW:
1. Thêm cờ liên kết thư viện mạng Windows: `-lws2_32` vào mục **Project -> Build Options -> Linker settings -> Other linker options**.
2. Khởi chạy `FTPServer`:
   - *Lưu ý*: Cổng 21 trên Windows có thể yêu cầu quyền Quản trị viên (Run as administrator).
   - Nhập `start` để bắt đầu lắng nghe.
3. Khởi chạy `FTPClient`:
   - Kết nối: `login 127.0.0.1 admin 1234`
   - Tải thư mục: `mget docs`
   - Thoát: `quit`

Xem hướng dẫn chi tiết từng bước tại [`ftp_v5/HUONG_DAN_CHAY.md`](ftp_v5/HUONG_DAN_CHAY.md).
