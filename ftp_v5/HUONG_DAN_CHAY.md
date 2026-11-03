# Hướng Dẫn Cài Đặt & Chạy – FTP v5 (De tai 13)

## Cấu Trúc File
```
ftp_v5/
├── FTPServer.cpp        ← Code server (C++ Winsock)
├── FTPClient.cpp        ← Code client (C++ Winsock)
├── server_files/
│   └── docs/
│       ├── file1.txt
│       ├── file2.txt
│       └── file3.txt
└── client_downloads/    ← File tải về lưu ở đây
```

---

## Bước 1: Tạo Project Trong CodeBlocks

### Tạo project SERVER:
1. File → New → Project → **Console Application** → C++
2. Tên project: `FTPServer`
3. Lưu vào thư mục `ftp_v5/`
4. Xóa file `main.cpp` mặc định
5. Kéo file `FTPServer.cpp` vào project

### Tạo project CLIENT:
1. Làm tương tự, tên: `FTPClient`
2. Kéo file `FTPClient.cpp` vào project

---

## Bước 2: Thêm Linker Options (BẮT BUỘC)

Cả 2 project đều phải làm bước này:

1. **Project** → **Build Options**
2. Chọn tab **Linker settings**
3. Phần **Other linker options**, thêm: `-lws2_32`
4. Nhấn **OK**

> Nếu không làm bước này sẽ bị lỗi: `undefined reference to WSAStartup`

---

## Bước 3: Chạy Chương Trình

### Chạy Server (cần quyền Admin vì cổng 21):
- Click phải vào **CodeBlocks.exe** → **Run as administrator**
- Build và chạy `FTPServer`
- Gõ `start`

> **Nếu bị lỗi bind cổng 21:** Đổi `#define CONG_FTP 21` thành `#define CONG_FTP 2121`
> trong cả 2 file (FTPServer.cpp và FTPClient.cpp)

### Chạy Client (Terminal khác):
- Build và chạy `FTPClient`
- Gõ: `login 127.0.0.1 admin 1234`
- Gõ: `mget docs`
- Gõ: `quit`

---

## Ví Dụ Chạy Thực Tế

### Terminal 1 - Server:
```
Initialising Winsock...Initialised.

========================================
      FTP SERVER - DE TAI 13
========================================
Lenh: start | stop | exit
----------------------------------------
server> start
Socket created.
Bind done.
[SERVER] Dang lang nghe tren cong 21...
server> [SERVER] Client 127.0.0.1 da ket noi
[SERVER] Nhan: USER admin
[SERVER] Nhan: PASS ****
[SERVER] Xac thuc thanh cong cho user 'admin'
[SERVER] Nhan lenh: LIST docs
[SERVER] Thu muc 'docs' co 3 file
[SERVER] Da gui xong danh sach file
[SERVER] Nhan lenh: RETR docs/file1.txt
[SERVER] Dang gui file: docs/file1.txt
[SERVER] Da gui xong file: docs/file1.txt
...
```

### Terminal 2 - Client:
```
Initialising Winsock...Initialised.

========================================
      FTP CLIENT - DE TAI 13
========================================
client> login 127.0.0.1 admin 1234
Socket created.
Connected.
[CLIENT] 220 FTP Server Ready
[CLIENT] 331 Password required
[CLIENT] 230 Login successful
[CLIENT] Dang nhap thanh cong!

client> mget docs
[CLIENT] Dang lay danh sach file trong thu muc 'docs'...
[CLIENT] Tim thay 3 file trong thu muc 'docs'
[CLIENT] Da tai 1/3 file: file1.txt
[CLIENT] Da tai 2/3 file: file2.txt
[CLIENT] Da tai 3/3 file: file3.txt
[CLIENT] Hoan thanh! Da tai 3/3 file ve thu muc 'client_downloads'

client> quit
[CLIENT] 221 Goodbye
[CLIENT] Da ngat ket noi.
```

