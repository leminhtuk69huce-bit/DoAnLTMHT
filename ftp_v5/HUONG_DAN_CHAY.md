# Hướng Dẫn Cài Đặt & Chạy – FTP v5 (Đề tài 13)

## Cấu Trúc Mã Nguồn (Chuẩn C++ .cpp & .h)
```
ftp_v5/
├── FTPServer.h          ← Khai báo thư viện & hàm Server
├── FTPServer.cpp        ← Triển khai logic Server (Winsock, đa luồng, FTP)
├── ServerMain.cpp       ← Hàm main() điều khiển Server
│
├── FTPClient.h          ← Khai báo thư viện & hàm Client
├── FTPClient.cpp        ← Triển khai logic Client (Winsock, tải file, mget)
├── ClientMain.cpp       ← Hàm main() điều khiển Client
│
├── server_files/        ← Thư mục dữ liệu Server (chứa docs/file1.txt,...)
└── client_downloads/    ← Thư mục lưu kết quả tải về
```

---

## Bước 1: Tạo Project Trong CodeBlocks

### Tạo project SERVER:
1. File → New → Project → **Console Application** → C++
2. Tên project: `FTPServer`, lưu vào thư mục `ftp_v5/`
3. Xóa file `main.cpp` mặc định mà Code::Blocks tự tạo
4. Thêm 3 file vào project:
   - `FTPServer.h`
   - `FTPServer.cpp`
   - `ServerMain.cpp`

### Tạo project CLIENT:
1. File → New → Project → **Console Application** → C++
2. Tên project: `FTPClient`, lưu vào thư mục `ftp_v5/`
3. Xóa file `main.cpp` mặc định
4. Thêm 3 file vào project:
   - `FTPClient.h`
   - `FTPClient.cpp`
   - `ClientMain.cpp`

---

## Bước 2: Thêm Linker Options (BẮT BUỘC)

Cả 2 project đều phải cấu hình bước này:

1. Vào menu: **Project** → **Build Options**
2. Chọn tab **Linker settings**
3. Trong ô **Other linker options**, điền:
   ```text
   -lws2_32 -static
   ```
4. Nhấn **OK**

> **Giải thích tham số:**
> - `-lws2_32`: Liên kết thư viện mạng Windows Winsock2 (tránh lỗi `undefined reference to WSAStartup`).
> - `-static`: Đóng gói thư viện C++ trực tiếp vào file `.exe` (giúp file chạy được trên mọi máy tính mà không lo thiếu file DLL).

---

## Bước 3: Chạy & Trải Nghiệm Chương Trình

### 1. Khởi động Server:
- Chạy với quyền **Run as administrator** (do cổng 21 là cổng hệ thống).
- Nhập: `start`
- Server sẽ tự động dò và in địa chỉ IP của máy ra màn hình:
  ```text
  server> start
  Socket created.
  Bind done.
  [SERVER] Dang lang nghe tren cong 21...
  [SERVER] Dia chi IP de Client ket noi:
    - Cung may (Localhost) : 127.0.0.1
    - Mang LAN (May khac)  : 192.168.1.168
    => Cu phap ben Client : login <IP> admin 1234
  ```
- **Tạo thêm tài khoản mới cho người dùng (Mới):**
  ```text
  server> adduser sinhvien 123456
  [SERVER] Da tao/cap nhat tai khoan 'sinhvien' thanh cong!
  ```
- **Xem danh sách tài khoản hiện có:**
  ```text
  server> users
  ```

### 2. Khởi động Client:
- Mở cửa sổ Client.
- **Đăng nhập:**
  ```text
  client> login 127.0.0.1 admin 1234
  ```
  *(hoặc dùng tài khoản vừa tạo: `login 127.0.0.1 sinhvien 123456`)*

- **Xem danh sách file trong thư mục trước khi tải (Mới):**
  ```text
  client> list docs
  ```
  *(hoặc gõ tắt: `ls docs`)*

- **Tải một file đơn lẻ (Mới):**
  ```text
  client> get docs/file1.txt
  ```
  *(hoặc `get docs\file1.txt`)*

- **Tải toàn bộ thư mục (Bulk Download - mget):**
  ```text
  client> mget docs
  ```
  *(hoặc truyền 1 file cụ thể: `mget docs/file1.txt`)*
  *(Tiến trình tải từng file sẽ hiển thị và lưu vào `client_downloads/`).*

- **Thoát:**
  ```text
  client> quit
  ```
