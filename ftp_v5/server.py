"""
server.py - Chương trình Server cho đề tài FTP v5 (Bulk Download)
=================================================================
Chức năng: Lắng nghe kết nối từ client trên cổng 21, xử lý các lệnh
           USER, PASS, LIST, RETR, QUIT theo giao thức FTP đơn giản.
Lệnh điều khiển: start (khởi động) | stop (tắt server)
"""

import socket
import threading
import os


SERVER_PORT  = 21                   # Cổng điều khiển FTP chuẩn
SERVER_DIR   = "server_files"       # Thư mục gốc chứa file của server
USERS        = {"admin": "1234"}    # Tài khoản hợp lệ (username: password)

server_socket = None                # Socket chính đang lắng nghe
is_running    = False               # Trạng thái server (True = đang chạy)



def gui_phan_hoi(conn, thong_bao):
    """
    Gửi một dòng phản hồi tới client.
    Luôn thêm '\\r\\n' ở cuối theo chuẩn FTP.
    """
    conn.sendall((thong_bao + "\r\n").encode("utf-8"))


def nhan_lenh(conn):
    """
    Nhận một dòng lệnh từ client.
    Đọc từng byte cho đến khi gặp '\\r\\n' (kết thúc lệnh FTP).
    Trả về chuỗi đã loại bỏ '\\r\\n'.
    """
    du_lieu = b""
    while not du_lieu.endswith(b"\r\n"):
        byte = conn.recv(1)         # đọc 1 byte mỗi lần
        if not byte:
            break                   # kết nối bị đứt
        du_lieu += byte
    return du_lieu.decode("utf-8").strip()



def tao_data_server():
    """
    Tạo một socket tạm thời để nhận Data Connection từ client.
    Dùng port=0 để hệ điều hành tự cấp cổng trống.

    Trả về: (data_server_socket, so_cong)
    """
    data_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    data_server.bind(("", 0))          # 0 = hệ điều hành tự chọn cổng
    data_server.listen(1)
    so_cong = data_server.getsockname()[1]   # lấy cổng vừa được cấp
    return data_server, so_cong

def xu_ly_xac_thuc(conn):
    """
    Xử lý bước đăng nhập: nhận lệnh USER rồi PASS từ client.
    Trả về True nếu đăng nhập thành công, False nếu thất bại.
    """
    # Bước 1: Nhận lệnh USER
    lenh = nhan_lenh(conn)
    print(f"[SERVER] Nhận lệnh: {lenh}")

    if not lenh.startswith("USER "):
        gui_phan_hoi(conn, "500 Lệnh không hợp lệ")
        return False

    ten_nguoi_dung = lenh[5:]           # bỏ chữ "USER " ở đầu
    gui_phan_hoi(conn, "331 Password required")

    # Bước 2: Nhận lệnh PASS
    lenh = nhan_lenh(conn)
    print(f"[SERVER] Nhận lệnh: PASS ****")

    if not lenh.startswith("PASS "):
        gui_phan_hoi(conn, "500 Lệnh không hợp lệ")
        return False

    mat_khau = lenh[5:]                 # bỏ chữ "PASS " ở đầu

    # Kiểm tra tài khoản
    if USERS.get(ten_nguoi_dung) == mat_khau:
        gui_phan_hoi(conn, "230 Login successful")
        print(f"[SERVER] Xác thực thành công cho user '{ten_nguoi_dung}'")
        return True
    else:
        gui_phan_hoi(conn, "530 Login incorrect")
        print(f"[SERVER] Xác thực thất bại cho user '{ten_nguoi_dung}'")
        return False


def xu_ly_list(conn, server_ip, ten_thu_muc):
    """
    Xử lý lệnh LIST <tên_thư_mục>:
    - Kiểm tra thư mục có tồn tại không
    - Tạo Data Connection, gửi danh sách tên file qua đó
    """
    # Xây dựng đường dẫn thực trên server
    duong_dan = os.path.join(SERVER_DIR, ten_thu_muc)

    # Kiểm tra thư mục có tồn tại không
    if not os.path.isdir(duong_dan):
        gui_phan_hoi(conn, "550 Directory not found")
        print(f"[SERVER] Thư mục '{ten_thu_muc}' không tồn tại")
        return

    # Lấy danh sách file (chỉ file, không lấy thư mục con)
    ds_file = [f for f in os.listdir(duong_dan)
               if os.path.isfile(os.path.join(duong_dan, f))]

    print(f"[SERVER] Thư mục '{ten_thu_muc}' có {len(ds_file)} file: {ds_file}")

    # Tạo Data Connection
    data_server, cong = tao_data_server()
    gui_phan_hoi(conn, f"150 {cong} Opening data connection")

    # Chờ client kết nối vào Data Connection
    data_conn, _ = data_server.accept()

    # Gửi danh sách file: mỗi tên file cách nhau bằng \r\n
    noi_dung = "\r\n".join(ds_file) + "\r\n"
    data_conn.sendall(noi_dung.encode("utf-8"))

    # Đóng Data Connection sau khi gửi xong
    data_conn.close()
    data_server.close()

    # Thông báo hoàn thành
    gui_phan_hoi(conn, "226 Transfer complete")
    print(f"[SERVER] Đã gửi danh sách file thư mục '{ten_thu_muc}'")


# ─────────────────────────────────────────
# XỬ LÝ LỆNH RETR (Gửi nội dung file)
# ─────────────────────────────────────────

def xu_ly_retr(conn, server_ip, duong_dan_file):
    """
    Xử lý lệnh RETR <thư_mục>/<tên_file>:
    - Kiểm tra file có tồn tại không
    - Tạo Data Connection, gửi toàn bộ nội dung file (bytes) qua đó
    """
    # Xây dựng đường dẫn thực của file trên server
    duong_dan_day_du = os.path.join(SERVER_DIR, duong_dan_file)

    # Kiểm tra file có tồn tại không
    if not os.path.isfile(duong_dan_day_du):
        gui_phan_hoi(conn, "550 File not found")
        print(f"[SERVER] File '{duong_dan_file}' không tồn tại")
        return

    print(f"[SERVER] Đang gửi file: {duong_dan_file}")

    # Tạo Data Connection
    data_server, cong = tao_data_server()
    gui_phan_hoi(conn, f"150 {cong} Opening data connection")

    # Chờ client kết nối vào Data Connection
    data_conn, _ = data_server.accept()

    # Đọc và gửi nội dung file dưới dạng bytes
    with open(duong_dan_day_du, "rb") as f:
        while True:
            khoi = f.read(4096)     # đọc từng 4KB
            if not khoi:
                break               # đọc hết file
            data_conn.sendall(khoi)

    # Đóng Data Connection sau khi gửi xong
    data_conn.close()
    data_server.close()

    # Thông báo hoàn thành
    gui_phan_hoi(conn, "226 Transfer complete")
    print(f"[SERVER] Đã gửi xong file: {duong_dan_file}")


# ─────────────────────────────────────────
# VÒNG LẶP XỬ LÝ TỪNG CLIENT
# ─────────────────────────────────────────


def xu_ly_client(conn, addr):
    """
    Xử lý toàn bộ phiên làm việc với một client.
    Hàm này chạy trong thread riêng cho mỗi client.
    """
    server_ip = conn.getsockname()[0]   # lấy IP của server

    try:
        # Gửi thông báo chào mừng ngay khi client kết nối
        gui_phan_hoi(conn, "220 FTP Server Ready")

        # Bước 1: Xác thực (USER + PASS)
        if not xu_ly_xac_thuc(conn):
            return      # xác thực thất bại, đóng kết nối

        # Bước 2: Vòng lặp nhận lệnh từ client
        while True:
            lenh = nhan_lenh(conn)
            if not lenh:
                break   # kết nối bị đứt

            print(f"[SERVER] Nhận lệnh từ {addr[0]}: {lenh}")

            # Xử lý lệnh LIST <tên_thư_mục>
            if lenh.startswith("LIST "):
                ten_thu_muc = lenh[5:]      # bỏ chữ "LIST " ở đầu
                xu_ly_list(conn, server_ip, ten_thu_muc)

            # Xử lý lệnh RETR <thư_mục>/<tên_file>
            elif lenh.startswith("RETR "):
                duong_dan = lenh[5:]        # bỏ chữ "RETR " ở đầu
                xu_ly_retr(conn, server_ip, duong_dan)

            # Xử lý lệnh QUIT
            elif lenh == "QUIT":
                gui_phan_hoi(conn, "221 Goodbye")
                print(f"[SERVER] Client {addr[0]} đã ngắt kết nối")
                break

            # Lệnh không nhận ra
            else:
                gui_phan_hoi(conn, "500 Unknown command")

    except (ConnectionResetError, BrokenPipeError):
        print(f"[SERVER] Kết nối với {addr[0]} bị đứt đột ngột")
    finally:
        conn.close()    # luôn đóng socket khi xong


# ─────────────────────────────────────────
# VÒNG LẶP CHẤP NHẬN KẾT NỐI (CHẠY TRONG THREAD)
# ─────────────────────────────────────────

def vong_lap_accept():
    """
    Vòng lặp chờ kết nối từ client.
    Chạy trong thread riêng để không block chương trình chính.
    Mỗi client được xử lý trong một thread riêng.
    """
    global is_running
    while is_running:
        try:
            conn, addr = server_socket.accept()
            print(f"[SERVER] Client {addr[0]}:{addr[1]} đã kết nối")

            # Tạo thread riêng để xử lý client này
            t = threading.Thread(target=xu_ly_client, args=(conn, addr))
            t.daemon = True         # thread tự tắt khi server tắt
            t.start()

        except OSError:
            # Socket bị đóng (do lệnh stop) → thoát vòng lặp
            break


# ─────────────────────────────────────────
# LỆNH START / STOP
# ─────────────────────────────────────────

def khoi_dong_server():
    """
    Khởi động server: tạo socket, bind cổng 21, bắt đầu lắng nghe.
    Chạy vòng lặp accept trong một thread riêng.
    """
    global server_socket, is_running

    if is_running:
        print("[SERVER] Server đã đang chạy rồi!")
        return

    # Tạo TCP socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind(("", SERVER_PORT))
    server_socket.listen(5)

    is_running = True
    print(f"[SERVER] Đang lắng nghe trên cổng {SERVER_PORT}...")

    # Chạy vòng lặp accept trong thread riêng
    t = threading.Thread(target=vong_lap_accept)
    t.daemon = True
    t.start()


def tat_server():
    """
    Tắt server: đổi trạng thái và đóng socket chính.
    """
    global server_socket, is_running

    if not is_running:
        print("[SERVER] Server chưa chạy!")
        return

    is_running = False
    server_socket.close()
    print("[SERVER] Đã tắt server.")


# ─────────────────────────────────────────
# CHƯƠNG TRÌNH CHÍNH
# ─────────────────────────────────────────

def main():
    print("=" * 40)
    print("       FTP SERVER - ĐỀ TÀI 13")
    print("=" * 40)
    print("Lệnh: start | stop | exit")
    print("-" * 40)

    while True:
        lenh = input("> ").strip().lower()

        if lenh == "start":
            khoi_dong_server()

        elif lenh == "stop":
            tat_server()

        elif lenh == "exit":
            if is_running:
                tat_server()
            print("[SERVER] Thoát chương trình.")
            break

        else:
            print("Lệnh không hợp lệ. Dùng: start | stop | exit")


if __name__ == "__main__":
    main()

