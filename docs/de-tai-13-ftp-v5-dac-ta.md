# 📋 Đặc Tả Chi Tiết – Đề Tài 13: FTP v5 (Bulk Download)

> [!IMPORTANT]
> **Mục tiêu tài liệu này**: Đây là bản đặc tả kỹ thuật đầy đủ để AI (hoặc lập trình viên) có thể viết code chính xác, dễ hiểu theo đúng yêu cầu đề bài mà không cần hỏi thêm.

---

## 1. Tổng Quan Bài Toán

| Mục | Nội dung |
|---|---|
| **Tên đề tài** | FTP v5 – Tải toàn bộ thư mục (Bulk Download) |
| **Mô hình** | Client – Server (2 chương trình riêng biệt) |
| **Giao thức** | FTP (mô phỏng, không dùng thư viện FTP có sẵn) |
| **Ngôn ngữ** | Python (khuyến nghị, dễ hiểu, ít boilerplate) |
| **Cổng Control** | 21 (Control Connection – gửi lệnh) |
| **Cổng Data** | Cổng ngẫu nhiên do server cấp (Data Connection – truyền file) |

### Luồng hoạt động tổng thể:
```
[Client]                          [Server]
   |                                  |
   |--- TCP connect port 21 -------> |  (Control Connection)
   |--- USER <username> -----------> |
   |--- PASS <password> -----------> |
   |<-- 230 Login OK --------------- |
   |                                  |
   |--- LIST <directory> ----------> |
   |<-- 150 Opening data conn ------- |
   |<-- [data: danh sách file] ------ |  (qua Data Connection mới)
   |<-- 226 Transfer complete ------- |
   |                                  |
   |  (lặp cho từng file trong danh sách)
   |--- RETR <filename> -----------> |
   |<-- 150 Opening data conn ------- |
   |<-- [data: nội dung file] ------- |  (qua Data Connection mới)
   |<-- 226 Transfer complete ------- |
   |  (hiển thị: "Đã tải 1/5 file...")
   |                                  |
   |--- QUIT -----------------------> |
   |<-- 221 Goodbye ----------------- |
   |--- đóng kết nối --------------- |
```

---

## 2. Cấu Trúc File Dự Án

```
ftp_v5/
│
├── server.py          # Chương trình Server
├── client.py          # Chương trình Client
│
├── server_files/      # Thư mục chứa file của server (tạo sẵn để test)
│   └── docs/
│       ├── file1.txt
│       ├── file2.txt
│       └── image.png
│
└── client_downloads/  # Thư mục lưu file tải về phía client (tạo sẵn)
```

---

## 3. Đặc Tả Chi Tiết: SERVER (`server.py`)

### 3.1 Cách chạy Server
```bash
python server.py
```
Sau khi chạy, server hiển thị menu:
```
=== FTP SERVER ===
Nhập lệnh: start | stop
> start
[SERVER] Đang lắng nghe trên cổng 21...
> stop
[SERVER] Đã tắt.
```

### 3.2 Lệnh `start`
- Tạo một **TCP socket** lắng nghe trên cổng **21**
- Chạy vòng lặp **accept** trong một **luồng (thread) riêng** để không block main thread
- Mỗi khi có client kết nối, tạo một thread mới để xử lý client đó
- Ghi log ra màn hình: `[SERVER] Client <IP>:<PORT> đã kết nối`

### 3.3 Lệnh `stop`
- Dừng vòng lặp accept
- Đóng socket chính
- Ghi log: `[SERVER] Đã tắt`

### 3.4 Xử lý từng Client (trong thread riêng)

Server nhận lệnh từ client theo thứ tự, mỗi lệnh là một dòng text (kết thúc `\r\n`):

#### Bước 1: Chào hỏi
- Khi client kết nối xong, server gửi ngay: `220 FTP Server Ready\r\n`

#### Bước 2: Xác thực
```
Client gửi:  USER <username>\r\n
Server trả:  331 Password required\r\n

Client gửi:  PASS <password>\r\n
Server trả:  230 Login successful\r\n   (nếu đúng)
             530 Login incorrect\r\n    (nếu sai)
```
- **Tài khoản hợp lệ**: username = `admin`, password = `1234` (hardcode đơn giản)
- Nếu sai, đóng kết nối

#### Bước 3: Nhận lệnh LIST
```
Client gửi:  LIST <tên_thư_mục>\r\n
```
Server xử lý:
1. Kiểm tra thư mục `server_files/<tên_thư_mục>` có tồn tại không
2. **Nếu không tồn tại**: gửi `550 Directory not found\r\n`, kết thúc
3. **Nếu tồn tại**:
   - Lấy danh sách tên file (không bao gồm thư mục con) trong thư mục đó
   - Mở một **server socket tạm** trên cổng bất kỳ (dùng port=0, hệ điều hành tự cấp)
   - Lấy cổng đó (ví dụ: 54321)
   - Gửi cho client: `150 <PORT> Opening data connection\r\n` (trong đó `<PORT>` là số cổng)
   - **Accept** kết nối data từ client
   - Gửi danh sách file qua data connection, định dạng: mỗi tên file một dòng, kết thúc bằng dòng trống `\r\n`
   - Đóng data connection
   - Gửi: `226 Transfer complete\r\n`

#### Bước 4: Nhận lệnh RETR (lặp nhiều lần, mỗi file một lần)
```
Client gửi:  RETR <tên_thư_mục>/<tên_file>\r\n
```
Server xử lý:
1. Đọc nội dung file tại đường dẫn `server_files/<tên_thư_mục>/<tên_file>`
2. Mở **server socket tạm** trên cổng bất kỳ
3. Gửi cho client: `150 <PORT> Opening data connection\r\n`
4. **Accept** kết nối data từ client
5. Gửi toàn bộ nội dung file (dưới dạng **bytes**) qua data connection
6. Đóng data connection
7. Gửi: `226 Transfer complete\r\n`

#### Bước 5: Nhận lệnh QUIT
```
Client gửi:  QUIT\r\n
Server trả:  221 Goodbye\r\n
```
Sau đó đóng control connection và kết thúc thread.

---

## 4. Đặc Tả Chi Tiết: CLIENT (`client.py`)

### 4.1 Cách chạy Client
```bash
python client.py
```
Sau khi chạy, client hiển thị menu:
```
=== FTP CLIENT ===
Nhập lệnh: login | mget | quit
> login 127.0.0.1 admin 1234
> mget docs
> quit
```

### 4.2 Lệnh `login <IP_server> <username> <password>`

Các bước client thực hiện:
1. Tạo **TCP socket** và kết nối tới `<IP_server>:21`
2. Nhận chuỗi chào: `220 FTP Server Ready\r\n` — in ra màn hình
3. Gửi: `USER <username>\r\n`
4. Nhận phản hồi `331 ...` — in ra màn hình
5. Gửi: `PASS <password>\r\n`
6. Nhận phản hồi:
   - `230 ...` → in `[CLIENT] Đăng nhập thành công!`, lưu socket để dùng tiếp
   - `530 ...` → in `[CLIENT] Sai tài khoản/mật khẩu`, đóng socket

### 4.3 Lệnh `mget <tên_thư_mục>`

> Chỉ dùng được sau khi đã `login` thành công.

Các bước client thực hiện:

**Bước A – Lấy danh sách file:**
1. Gửi qua control connection: `LIST <tên_thư_mục>\r\n`
2. Nhận phản hồi từ server:
   - Nếu `550 ...` → in lỗi, dừng
   - Nếu `150 <PORT> ...` → lấy số cổng `<PORT>` từ chuỗi này
3. Tạo **TCP socket mới** (data connection), kết nối tới `<IP_server>:<PORT>`
4. Nhận toàn bộ dữ liệu từ data connection (danh sách file, mỗi dòng là một tên file)
5. Đóng data connection
6. Nhận `226 Transfer complete\r\n` từ control connection
7. Tách chuỗi nhận được thành danh sách: `file_list = ["file1.txt", "file2.txt", ...]`
8. In ra: `[CLIENT] Tìm thấy 3 file trong thư mục 'docs'`

**Bước B – Tải từng file (vòng lặp):**

Với mỗi `filename` trong `file_list` (đánh số `i` từ 1):
1. Gửi qua control connection: `RETR <tên_thư_mục>/<filename>\r\n`
2. Nhận phản hồi `150 <PORT> ...` → lấy cổng
3. Tạo **TCP socket mới** (data connection), kết nối tới `<IP_server>:<PORT>`
4. Nhận toàn bộ bytes từ data connection
5. Đóng data connection
6. Nhận `226 Transfer complete\r\n` từ control connection
7. Lưu file vào `client_downloads/<filename>`
8. In ra: `[CLIENT] Đã tải <i>/<tổng_số> file: <filename>`

Sau khi vòng lặp kết thúc:
```
[CLIENT] Hoàn thành! Đã tải 3/3 file về thư mục 'client_downloads/'
```

### 4.4 Lệnh `quit`
1. Gửi qua control connection: `QUIT\r\n`
2. Nhận `221 Goodbye\r\n`
3. Đóng socket
4. In: `[CLIENT] Đã ngắt kết nối.`
5. Thoát chương trình

---

## 5. Quy Tắc Lập Trình (Bắt Buộc Tuân Theo)

> [!NOTE]
> Các quy tắc này giúp code dễ đọc, dễ hiểu cho sinh viên.

### 5.1 Quy tắc chung
- **Mỗi hàm chỉ làm 1 việc** – không nhồi nhiều logic vào 1 hàm
- **Comment tiếng Việt** trên mỗi khối logic quan trọng
- **Tên biến, hàm rõ nghĩa**: `control_socket`, `data_socket`, `file_list`, `total_files`
- **Không dùng thư viện FTP** (`ftplib`) – phải tự xây dựng giao thức bằng socket thô
- **Xử lý lỗi** bằng `try/except` ở những chỗ có thể gây crash

### 5.2 Cách gửi/nhận lệnh (quan trọng!)
```python
# GỬI lệnh (luôn kết thúc bằng \r\n)
def send_command(sock, command):
    sock.sendall((command + "\r\n").encode("utf-8"))

# NHẬN phản hồi (đọc 1 dòng)
def receive_response(sock):
    response = b""
    while not response.endswith(b"\r\n"):
        chunk = sock.recv(1)
        if not chunk:
            break
        response += chunk
    return response.decode("utf-8").strip()
```

### 5.3 Cách trích xuất PORT từ chuỗi `150 <PORT> ...`
```python
# Ví dụ: response = "150 54321 Opening data connection"
def extract_port(response):
    parts = response.split()  # ["150", "54321", "Opening", ...]
    port = int(parts[1])      # lấy phần tử thứ 2
    return port
```

### 5.4 Cách tạo Data Connection phía server
```python
import socket

def create_data_server():
    """Tạo socket tạm để nhận data connection từ client"""
    data_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    data_server.bind(("", 0))          # cổng 0 = hệ điều hành tự chọn
    data_server.listen(1)
    port = data_server.getsockname()[1]  # lấy cổng được cấp
    return data_server, port
```

### 5.5 Cách gửi/nhận file (bytes)
```python
# SERVER gửi file
def send_file(data_conn, filepath):
    with open(filepath, "rb") as f:
        data_conn.sendfile(f)  # hoặc: data_conn.sendall(f.read())

# CLIENT nhận file
def receive_file(data_conn, save_path):
    with open(save_path, "wb") as f:
        while True:
            chunk = data_conn.recv(4096)  # nhận từng 4KB
            if not chunk:
                break
            f.write(chunk)
```

---

## 6. Cấu Trúc Hàm Chi Tiết

### 6.1 `server.py`

```
server.py
├── Hàm: send_command(sock, message)          # gửi lệnh/phản hồi
├── Hàm: receive_command(sock)                # nhận lệnh từ client
├── Hàm: create_data_server()                 # tạo data socket tạm
├── Hàm: handle_auth(sock)                    # xử lý USER/PASS → True/False
├── Hàm: handle_list(sock, server_ip, dir)    # xử lý lệnh LIST
├── Hàm: handle_retr(sock, server_ip, path)   # xử lý lệnh RETR
├── Hàm: handle_client(conn, addr)            # vòng lặp xử lý 1 client (chạy trong thread)
├── Hàm: start_server()                       # khởi động server
├── Hàm: stop_server()                        # tắt server
└── Hàm: main()                               # vòng lặp nhập lệnh start/stop
```

### 6.2 `client.py`

```
client.py
├── Hàm: send_command(sock, command)          # gửi lệnh tới server
├── Hàm: receive_response(sock)               # nhận phản hồi từ server
├── Hàm: extract_port(response)               # trích port từ chuỗi "150 PORT ..."
├── Hàm: login(ip, username, password)        # thực hiện đăng nhập
├── Hàm: mget(directory)                      # tải toàn bộ thư mục
│   ├── get_file_list(directory)              #   bước A: lấy danh sách file
│   └── download_file(dir, filename, idx, total) # bước B: tải từng file
├── Hàm: quit_session()                       # thoát phiên làm việc
└── Hàm: main()                               # vòng lặp nhập lệnh client
```

---

## 7. Biến Toàn Cục (Global State)

### Trong `server.py`:
```python
server_socket = None    # socket chính đang lắng nghe
is_running = False      # trạng thái server (đang chạy hay không)
SERVER_DIR = "server_files"  # thư mục gốc chứa file
USERS = {"admin": "1234"}   # tài khoản hợp lệ
```

### Trong `client.py`:
```python
control_socket = None   # socket kết nối điều khiển (port 21)
server_ip = None        # IP của server đang kết nối
DOWNLOAD_DIR = "client_downloads"  # thư mục lưu file tải về
```

---

## 8. Các Trường Hợp Lỗi Cần Xử Lý

| Trường hợp | Xử lý |
|---|---|
| Client nhập sai username/password | Server gửi `530`, client in thông báo lỗi |
| Thư mục không tồn tại trên server | Server gửi `550`, client in thông báo lỗi |
| Thư mục rỗng (không có file nào) | Client in `"Thư mục rỗng, không có file để tải"` |
| Gọi `mget` trước khi `login` | Client in `"Chưa đăng nhập! Dùng lệnh login trước."` |
| Kết nối bị ngắt giữa chừng | Bắt `ConnectionResetError`, in thông báo |

---

## 9. Ví Dụ Chạy Thực Tế

### Terminal 1 – Server:
```
=== FTP SERVER ===
> start
[SERVER] Đang lắng nghe trên cổng 21...
[SERVER] Client 127.0.0.1:52341 đã kết nối
[SERVER] Xác thực thành công cho user 'admin'
[SERVER] Gửi danh sách file thư mục 'docs' (3 file)
[SERVER] Đang gửi file: docs/file1.txt
[SERVER] Đang gửi file: docs/file2.txt
[SERVER] Đang gửi file: docs/image.png
[SERVER] Client 127.0.0.1:52341 đã ngắt kết nối
```

### Terminal 2 – Client:
```
=== FTP CLIENT ===
> login 127.0.0.1 admin 1234
[CLIENT] 220 FTP Server Ready
[CLIENT] 331 Password required
[CLIENT] Đăng nhập thành công!

> mget docs
[CLIENT] Tìm thấy 3 file trong thư mục 'docs'
[CLIENT] Đã tải 1/3 file: file1.txt
[CLIENT] Đã tải 2/3 file: file2.txt
[CLIENT] Đã tải 3/3 file: image.png
[CLIENT] Hoàn thành! Đã tải 3/3 file về thư mục 'client_downloads/'

> quit
[CLIENT] 221 Goodbye
[CLIENT] Đã ngắt kết nối.
```

---

## 10. Prompt Mẫu Dùng Cho AI Khác

Dưới đây là prompt bạn dán vào bất kỳ AI nào (ChatGPT, Gemini, Claude...) để nó viết code chính xác:

---

### 🤖 PROMPT CHO AI – VIẾT SERVER (`server.py`)

```
Viết file server.py bằng Python cho đề tài FTP v5 (Bulk Download).

YÊU CẦU KỸ THUẬT:
- Dùng thư viện: socket, threading, os (KHÔNG dùng ftplib)
- Server lắng nghe cổng 21 (Control Connection)
- Tài khoản hợp lệ: username="admin", password="1234" (hardcode)
- Thư mục gốc server: "server_files/" (phải tồn tại)

CÁC LỆNH SERVER XỬ LÝ:
1. Khi client kết nối: gửi "220 FTP Server Ready\r\n"
2. Nhận "USER <username>\r\n" → gửi "331 Password required\r\n"
3. Nhận "PASS <password>\r\n":
   - Đúng → gửi "230 Login successful\r\n"
   - Sai → gửi "530 Login incorrect\r\n" rồi đóng kết nối
4. Nhận "LIST <dir>\r\n":
   - Tạo data socket tạm (port=0, hệ điều hành tự cấp cổng)
   - Gửi "150 <PORT> Opening data connection\r\n"
   - Accept data connection từ client
   - Gửi danh sách tên file (mỗi file 1 dòng \r\n, kết thúc bằng \r\n rỗng)
   - Đóng data connection, gửi "226 Transfer complete\r\n"
5. Nhận "RETR <dir>/<filename>\r\n":
   - Tạo data socket tạm, gửi "150 <PORT> Opening data connection\r\n"
   - Accept data connection, gửi bytes của file
   - Đóng data connection, gửi "226 Transfer complete\r\n"
6. Nhận "QUIT\r\n" → gửi "221 Goodbye\r\n", đóng connection

LỆNH ĐIỀU KHIỂN (main):
- Gõ "start": chạy vòng lặp accept trong thread riêng, in "[SERVER] Đang lắng nghe trên cổng 21..."
- Gõ "stop": dừng server, đóng socket chính

QUY TẮC CODE:
- Comment tiếng Việt trên mỗi khối logic
- Mỗi hàm chỉ làm 1 việc, tên hàm rõ nghĩa
- Xử lý ngoại lệ bằng try/except
- Code đơn giản, dễ đọc (không dùng các pattern phức tạp)
```

---

### 🤖 PROMPT CHO AI – VIẾT CLIENT (`client.py`)

```
Viết file client.py bằng Python cho đề tài FTP v5 (Bulk Download).

YÊU CẦU KỸ THUẬT:
- Dùng thư viện: socket, os (KHÔNG dùng ftplib)
- Thư mục lưu file tải về: "client_downloads/" (tự tạo nếu chưa có)
- Biến global: control_socket = None, server_ip = None

CÁC LỆNH NGƯỜI DÙNG:

1. login <IP> <username> <password>:
   - Kết nối TCP tới <IP>:21
   - Nhận "220 ..." → in ra
   - Gửi "USER <username>\r\n", nhận "331 ..." → in ra
   - Gửi "PASS <password>\r\n", nhận phản hồi:
     + "230 ..." → in "[CLIENT] Đăng nhập thành công!", lưu socket
     + "530 ..." → in "[CLIENT] Sai tài khoản/mật khẩu", đóng socket

2. mget <tên_thư_mục>:
   - Kiểm tra đã login chưa (control_socket != None), nếu chưa in lỗi
   - BƯỚC A – Lấy danh sách file:
     + Gửi "LIST <tên_thư_mục>\r\n"
     + Nhận "150 <PORT> ..." → trích xuất PORT (split()[1])
     + Kết nối data TCP tới server_ip:<PORT>
     + Nhận toàn bộ bytes → decode → tách thành list tên file (split("\r\n"))
     + Đóng data socket, nhận "226 ..."
     + In "[CLIENT] Tìm thấy N file trong thư mục 'X'"
   - BƯỚC B – Tải từng file (for i, filename in enumerate(file_list)):
     + Gửi "RETR <tên_thư_mục>/<filename>\r\n"
     + Nhận "150 <PORT> ..." → trích xuất PORT
     + Kết nối data TCP tới server_ip:<PORT>
     + Nhận bytes → ghi vào "client_downloads/<filename>" (mode "wb")
     + Đóng data socket, nhận "226 ..."
     + In "[CLIENT] Đã tải <i+1>/<total> file: <filename>"
   - In "[CLIENT] Hoàn thành! Đã tải N/N file về thư mục 'client_downloads/'"

3. quit:
   - Gửi "QUIT\r\n", nhận "221 ...", đóng socket, in thông báo, thoát chương trình

VÒNG LẶP CHÍNH (main):
- In "=== FTP CLIENT ==="
- Vòng lặp while True: in "> ", nhận lệnh, phân tách và gọi hàm tương ứng

QUY TẮC CODE:
- Comment tiếng Việt trên mỗi khối logic
- Mỗi hàm chỉ làm 1 việc, tên hàm rõ nghĩa
- Xử lý ngoại lệ bằng try/except
- Code đơn giản, dễ đọc
```

---

## 11. Kiểm Tra Sau Khi Có Code

### Checklist kiểm tra:
- [ ] Server khởi động và lắng nghe cổng 21
- [ ] Client đăng nhập thành công với `admin/1234`
- [ ] Client đăng nhập thất bại với mật khẩu sai → báo lỗi đúng
- [ ] `mget docs` → liệt kê đúng số file trong thư mục
- [ ] Tất cả file được tải về `client_downloads/`
- [ ] Nội dung file tải về giống hệt file gốc (kể cả file nhị phân như .png)
- [ ] Tiến trình hiển thị đúng: `"Đã tải 1/3 file..."`
- [ ] `quit` đóng kết nối sạch sẽ
- [ ] Thư mục không tồn tại → báo lỗi, không crash

---

*Tài liệu được tạo cho Đề tài 13 – Môn Lập trình Hệ thống Mạng*
