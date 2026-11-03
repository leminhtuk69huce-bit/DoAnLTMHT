# ĐỀ TÀI 13: FTP v5 – BULK DOWNLOAD

> [!IMPORTANT]
> **Code đã viết xong và test pass 100%.** Bạn chỉ cần chạy theo hướng dẫn bên dưới.

---

## 📁 Cấu Trúc Dự Án

```
ftp_v5/
├── server.py              ✅ Hoàn chỉnh
├── client.py              ✅ Hoàn chỉnh
├── server_files/
│   └── docs/
│       ├── file1.txt      (file mẫu để test)
│       ├── file2.txt
│       └── file3.txt
└── client_downloads/      (file tải về được lưu ở đây)
```

---

## ▶️ Hướng Dẫn Chạy

### Bước 1 – Mở Terminal 1 (Server)
```bash
cd ftp_v5
python server.py
```
```
=== FTP SERVER - ĐỀ TÀI 13 ===
Lệnh: start | stop | exit
> start
[SERVER] Đang lắng nghe trên cổng 21...
```

> [!NOTE]
> Trên Windows, cổng 21 có thể yêu cầu chạy với quyền Administrator. Nếu bị lỗi, click chuột phải vào Terminal → "Run as Administrator" rồi chạy lại.

### Bước 2 – Mở Terminal 2 (Client)
```bash
cd ftp_v5
python client.py
```
```
=== FTP CLIENT - ĐỀ TÀI 13 ===
Lệnh:
  login <IP> <username> <password>
  mget <tên_thư_mục>
  quit
---
> login 127.0.0.1 admin 1234
[CLIENT] Đã kết nối tới server 127.0.0.1:21
[CLIENT] 220 FTP Server Ready
[CLIENT] 331 Password required
[CLIENT] 230 Login successful
[CLIENT] Đăng nhập thành công!

> mget docs
[CLIENT] Đang lấy danh sách file trong thư mục 'docs'...
[CLIENT] Tìm thấy 3 file trong thư mục 'docs'
[CLIENT] Đã tải 1/3 file: file1.txt
[CLIENT] Đã tải 2/3 file: file2.txt
[CLIENT] Đã tải 3/3 file: file3.txt
[CLIENT] Hoàn thành! Đã tải 3/3 file về thư mục 'client_downloads/'

> quit
[CLIENT] 221 Goodbye
[CLIENT] Đã ngắt kết nối.
```

---

## ✅ Kết Quả Test Đã Xác Nhận

| Bước | Lệnh | Phản hồi | Kết quả |
|---|---|---|---|
| 1 | Kết nối TCP | 220 FTP Server Ready | ✅ PASS |
| 2 | USER admin | 331 Password required | ✅ PASS |
| 3 | PASS 1234 | 230 Login successful | ✅ PASS |
| 4 | LIST docs | 150 PORT + danh sách 3 file | ✅ PASS |
| 5 | RETR docs/file1.txt | 150 PORT + bytes file | ✅ PASS |
| 6 | RETR docs/file2.txt | 150 PORT + bytes file | ✅ PASS |
| 7 | QUIT | 221 Goodbye | ✅ PASS |

File tải về **khớp hoàn toàn** với file gốc (byte-for-byte).

---

## 🤖 PROMPT CHO AI KHÁC – Làm Phần Thiết Kế Hệ Thống

> Dán đoạn dưới vào ChatGPT / Gemini / Claude để AI đó làm phần báo cáo thiết kế hệ thống.

---

### PROMPT ĐẦY ĐỦ (copy toàn bộ):

```
Tôi cần bạn viết phần THIẾT KẾ HỆ THỐNG cho báo cáo đồ án môn Lập trình 
Hệ thống Mạng. Đề tài: FTP v5 – Tải toàn bộ thư mục (Bulk Download).

============================
MÔ TẢ BÀI TOÁN
============================
Xây dựng ứng dụng Client–Server mô phỏng giao thức FTP phiên bản 5.
Client tải về TOÀN BỘ file trong một thư mục chỉ định trên server.

CÁC LỆNH HOẠT ĐỘNG:
Server:
  - start: Khởi động server, lắng nghe cổng 21
  - stop: Tắt server

Client:
  - login <IP> <username> <password>: Kết nối và đăng nhập FTP
  - mget <tên thư mục>: Tải toàn bộ file trong thư mục đó về máy
    (Hiển thị tiến trình: "Đã tải 1/5 file...")
  - quit: Kết thúc phiên, đóng kết nối

============================
GIAO THỨC SỬ DỤNG
============================
Mô phỏng FTP (không dùng thư viện ftplib).
Hai kết nối TCP:
  1. Control Connection – cổng 21, tồn tại suốt phiên, gửi lệnh/nhận phản hồi text
  2. Data Connection – cổng ngẫu nhiên do server cấp, tạo mới mỗi lần truyền

Mã phản hồi:
  220 – Server Ready (khi client kết nối)
  331 – Password required (sau USER)
  230 – Login successful (sau PASS đúng)
  530 – Login incorrect (sau PASS sai)
  150 <PORT> – Opening data connection (kèm số cổng)
  226 – Transfer complete
  550 – Directory/File not found
  221 – Goodbye (sau QUIT)

Luồng lệnh mget:
  Client gửi LIST <dir> → Server gửi 150 <PORT> → Client kết nối data 
  → Server gửi danh sách tên file (mỗi dòng 1 file, kết thúc \r\n)
  → Server đóng data → Server gửi 226
  → Client lặp: RETR <dir>/<file> → 150 <PORT> → data bytes file → 226

============================
CODE THỰC TẾ (để tham khảo)
============================
server.py có các hàm:
  - khoi_dong_server(): bind port 21, listen, chạy thread accept
  - tat_server(): đóng socket
  - xu_ly_client(conn, addr): vòng lặp xử lý từng client (thread)
  - xu_ly_xac_thuc(conn): xử lý USER + PASS
  - xu_ly_list(conn, ip, thu_muc): xử lý LIST
  - xu_ly_retr(conn, ip, duong_dan): xử lý RETR
  - tao_data_server(): tạo socket tạm cổng ngẫu nhiên

client.py có các hàm:
  - dang_nhap(ip, ten, pass): login
  - tai_toan_bo_thu_muc(ten_thu_muc): mget
  - lay_danh_sach_file(thu_muc): gửi LIST, nhận danh sách
  - tai_mot_file(thu_muc, file, idx, total): gửi RETR, nhận và lưu file
  - ket_thuc_phien(): gửi QUIT

============================
YÊU CẦU BẠN VIẾT
============================
Hãy viết phần thiết kế hệ thống gồm 4 mục sau, bằng tiếng Việt, 
dùng cho báo cáo đồ án sinh viên:

1. PHÁT BIỂU BÀI TOÁN
   - Mô tả yêu cầu chính (2-3 đoạn)
   - Bảng liệt kê các chức năng sẽ xây dựng

2. MÔ TẢ SƠ BỘ GIAO THỨC
   - Giải thích Control Connection và Data Connection
   - Bảng mã phản hồi FTP dùng trong đề tài
   - Vẽ sơ đồ luồng giao tiếp dạng ASCII art hoặc text

3. BIỂU ĐỒ USE CASE
   - Xác định các Actor (Quản trị viên Server, Người dùng Client)
   - Liệt kê các Use Case và mô tả từng cái
   - Vẽ biểu đồ Use Case (dùng PlantUML hoặc mô tả dạng text)

4. BIỂU ĐỒ SEQUENCE
   - Sequence diagram cho lệnh login
   - Sequence diagram cho lệnh mget (bao gồm LIST và nhiều RETR)
   - Sequence diagram cho lệnh quit
   (Dùng PlantUML hoặc mermaid)

5. BIỂU ĐỒ CLASS
   - Class FTPServer với đầy đủ thuộc tính và phương thức
   - Class FTPClient với đầy đủ thuộc tính và phương thức
   - Mô tả quan hệ giữa các class
   (Dùng PlantUML hoặc mermaid)

Lưu ý: Viết rõ ràng, phù hợp trình độ sinh viên đại học năm 3.
```

---

## 📝 Phần Bạn Cần Tự Làm Trong Báo Cáo

Sau khi có code chạy được, bạn cần viết mục **"Mô tả chi tiết cách lập trình"** trong báo cáo. Nội dung gợi ý:

### Mục: Mô tả cách lập trình thực thi các chức năng

**1. Cơ chế gửi/nhận lệnh (Control Connection)**
- Giải thích: mỗi lệnh là 1 dòng text kết thúc `\r\n`
- Hàm `gui_lenh()` và `nhan_phan_hoi()` trong client
- Hàm `gui_phan_hoi()` và `nhan_lenh()` trong server

**2. Cơ chế Data Connection**
- Hàm `tao_data_server()` – server tạo socket tạm, lấy cổng ngẫu nhiên
- Server gửi cổng qua `150 <PORT>`, client đọc cổng bằng `lay_so_cong()`
- Client kết nối vào cổng đó bằng `ket_noi_data()`

**3. Xử lý lệnh LIST**
- Server đọc thư mục bằng `os.listdir()`, lọc chỉ lấy file
- Gửi danh sách qua Data Connection, mỗi file 1 dòng
- Client tách chuỗi bằng `.split("\r\n")` để có list

**4. Xử lý lệnh RETR (vòng lặp mget)**
- Mỗi file: 1 lần tạo Data Connection riêng
- Server đọc file bằng `open(path, "rb")`, gửi từng chunk 4KB
- Client nhận bytes, ghi vào file bằng `open(path, "wb")`
- Hiển thị tiến trình `i/total`

**5. Đa luồng phía Server**
- `threading.Thread` cho vòng lặp accept
- Mỗi client kết nối → 1 thread riêng
- `daemon=True` để thread tự tắt khi server tắt
