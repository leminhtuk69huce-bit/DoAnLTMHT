"""
client.py - Chương trình Client cho đề tài FTP v5 (Bulk Download)
=================================================================
Chức năng: Kết nối tới server FTP, đăng nhập, tải toàn bộ file
           trong một thư mục chỉ định về máy local.
Lệnh: login <IP> <username> <password> | mget <thư_mục> | quit
"""

import socket
import os

# ─────────────────────────────────────────
# CÀI ĐẶT TOÀN CỤC
# ─────────────────────────────────────────
SERVER_PORT   = 21                   # Cổng điều khiển FTP chuẩn
DOWNLOAD_DIR  = "client_downloads"   # Thư mục lưu file tải về

control_socket = None                # Socket kết nối điều khiển (cổng 21)
server_ip      = None                # IP của server đang kết nối

# Tạo thư mục lưu file nếu chưa có
os.makedirs(DOWNLOAD_DIR, exist_ok=True)


# ─────────────────────────────────────────
# HÀM TIỆN ÍCH: Gửi / Nhận qua socket
# ─────────────────────────────────────────

def gui_lenh(lenh):
    """
    Gửi một lệnh tới server qua Control Connection.
    Luôn thêm '\\r\\n' ở cuối theo chuẩn FTP.
    """
    control_socket.sendall((lenh + "\r\n").encode("utf-8"))


def nhan_phan_hoi():
    """
    Nhận một dòng phản hồi từ server qua Control Connection.
    Đọc từng byte cho đến khi gặp '\\r\\n'.
    Trả về chuỗi đã loại bỏ '\\r\\n'.
    """
    du_lieu = b""
    while not du_lieu.endswith(b"\r\n"):
        byte = control_socket.recv(1)   # đọc 1 byte mỗi lần
        if not byte:
            break                       # kết nối bị đứt
        du_lieu += byte
    return du_lieu.decode("utf-8").strip()


def lay_so_cong(phan_hoi):
    """
    Trích xuất số cổng từ phản hồi '150 <PORT> Opening data connection'.
    Ví dụ: '150 54321 Opening...' → 54321

    Cách làm: tách chuỗi theo khoảng trắng, lấy phần tử thứ 2 (index 1).
    """
    phan = phan_hoi.split()     # ["150", "54321", "Opening", ...]
    return int(phan[1])         # lấy "54321" rồi chuyển sang số nguyên


def ket_noi_data(so_cong):
    """
    Tạo một Data Connection mới tới server tại cổng chỉ định.
    Mỗi lần LIST hoặc RETR đều tạo một kết nối data riêng.
    """
    data_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    data_sock.connect((server_ip, so_cong))
    return data_sock


# ─────────────────────────────────────────
# LỆNH LOGIN
# ─────────────────────────────────────────

def dang_nhap(ip, ten_nd, mat_khau):
    """
    Kết nối tới server và thực hiện xác thực theo giao thức FTP.
    Các bước: kết nối TCP → nhận 220 → gửi USER → gửi PASS → kiểm tra 230
    """
    global control_socket, server_ip

    # Bước 1: Kết nối TCP tới server cổng 21
    try:
        control_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        control_socket.connect((ip, SERVER_PORT))
        server_ip = ip
        print(f"[CLIENT] Đã kết nối tới server {ip}:{SERVER_PORT}")
    except ConnectionRefusedError:
        print(f"[CLIENT] Không thể kết nối tới {ip}:{SERVER_PORT}. Server đã chạy chưa?")
        control_socket = None
        return

    # Bước 2: Nhận thông báo chào mừng từ server (220 ...)
    phan_hoi = nhan_phan_hoi()
    print(f"[CLIENT] {phan_hoi}")

    # Bước 3: Gửi tên đăng nhập
    gui_lenh(f"USER {ten_nd}")
    phan_hoi = nhan_phan_hoi()
    print(f"[CLIENT] {phan_hoi}")    # "331 Password required"

    # Bước 4: Gửi mật khẩu
    gui_lenh(f"PASS {mat_khau}")
    phan_hoi = nhan_phan_hoi()
    print(f"[CLIENT] {phan_hoi}")    # "230 Login successful" hoặc "530 ..."

    # Bước 5: Kiểm tra kết quả xác thực
    if phan_hoi.startswith("230"):
        print("[CLIENT] Đăng nhập thành công!")
    else:
        print("[CLIENT] Đăng nhập thất bại. Kiểm tra lại tài khoản/mật khẩu.")
        control_socket.close()
        control_socket = None
        server_ip = None


# ─────────────────────────────────────────
# LỆNH MGET – BƯỚC A: LẤY DANH SÁCH FILE
# ─────────────────────────────────────────

def lay_danh_sach_file(ten_thu_muc):
    """
    Gửi lệnh LIST tới server, nhận danh sách tên file trong thư mục đó.
    Trả về danh sách tên file (list of str), hoặc [] nếu thất bại.
    """
    # Gửi lệnh LIST qua Control Connection
    gui_lenh(f"LIST {ten_thu_muc}")
    phan_hoi = nhan_phan_hoi()   # nhận "150 <PORT> ..." hoặc "550 ..."

    # Kiểm tra nếu thư mục không tồn tại
    if phan_hoi.startswith("550"):
        print(f"[CLIENT] Lỗi: Thư mục '{ten_thu_muc}' không tồn tại trên server.")
        return []

    # Trích xuất cổng Data Connection từ phản hồi "150 <PORT> ..."
    so_cong = lay_so_cong(phan_hoi)

    # Kết nối vào Data Connection để nhận danh sách file
    data_sock = ket_noi_data(so_cong)
    du_lieu_nhan = b""
    while True:
        khoi = data_sock.recv(4096)     # nhận từng 4KB
        if not khoi:
            break                       # server đóng data connection → nhận xong
        du_lieu_nhan += khoi
    data_sock.close()

    # Nhận thông báo "226 Transfer complete" từ Control Connection
    nhan_phan_hoi()

    # Tách chuỗi nhận được thành danh sách tên file
    # Ví dụ: "file1.txt\r\nfile2.txt\r\n" → ["file1.txt", "file2.txt"]
    ds_file = [f for f in du_lieu_nhan.decode("utf-8").split("\r\n") if f]
    return ds_file


# ─────────────────────────────────────────
# LỆNH MGET – BƯỚC B: TẢI MỘT FILE
# ─────────────────────────────────────────

def tai_mot_file(ten_thu_muc, ten_file, so_thu_tu, tong_so):
    """
    Gửi lệnh RETR để tải một file cụ thể từ server về máy client.
    Hiển thị tiến trình: "Đã tải 1/3 file: file1.txt"
    """
    # Gửi lệnh RETR qua Control Connection
    gui_lenh(f"RETR {ten_thu_muc}/{ten_file}")
    phan_hoi = nhan_phan_hoi()   # nhận "150 <PORT> ..."

    if phan_hoi.startswith("550"):
        print(f"[CLIENT] Lỗi: Không thể tải file '{ten_file}'")
        return

    # Trích xuất cổng Data Connection
    so_cong = lay_so_cong(phan_hoi)

    # Kết nối vào Data Connection để nhận nội dung file
    data_sock = ket_noi_data(so_cong)
    noi_dung_file = b""
    while True:
        khoi = data_sock.recv(4096)     # nhận từng 4KB
        if not khoi:
            break                       # server đóng data connection → nhận xong
        noi_dung_file += khoi
    data_sock.close()

    # Nhận thông báo "226 Transfer complete"
    nhan_phan_hoi()

    # Lưu file vào thư mục client_downloads/
    duong_dan_luu = os.path.join(DOWNLOAD_DIR, ten_file)
    with open(duong_dan_luu, "wb") as f:
        f.write(noi_dung_file)

    # Hiển thị tiến trình tải
    print(f"[CLIENT] Đã tải {so_thu_tu}/{tong_so} file: {ten_file}")


# ─────────────────────────────────────────
# LỆNH MGET – TỔNG HỢP
# ─────────────────────────────────────────

def tai_toan_bo_thu_muc(ten_thu_muc):
    """
    Thực hiện lệnh mget: tải toàn bộ file trong thư mục chỉ định.
    Bước A: Lấy danh sách file → Bước B: Tải từng file.
    """
    # Kiểm tra đã đăng nhập chưa
    if control_socket is None:
        print("[CLIENT] Chưa đăng nhập! Dùng lệnh: login <IP> <username> <password>")
        return

    # BƯỚC A: Lấy danh sách file trong thư mục
    print(f"[CLIENT] Đang lấy danh sách file trong thư mục '{ten_thu_muc}'...")
    ds_file = lay_danh_sach_file(ten_thu_muc)

    if not ds_file:
        print(f"[CLIENT] Thư mục '{ten_thu_muc}' rỗng hoặc không tồn tại.")
        return

    tong_so = len(ds_file)
    print(f"[CLIENT] Tìm thấy {tong_so} file trong thư mục '{ten_thu_muc}'")

    # BƯỚC B: Tải từng file trong danh sách
    for thu_tu, ten_file in enumerate(ds_file, start=1):
        tai_mot_file(ten_thu_muc, ten_file, thu_tu, tong_so)

    # Thông báo hoàn thành
    print(f"[CLIENT] Hoàn thành! Đã tải {tong_so}/{tong_so} file"
          f" về thư mục '{DOWNLOAD_DIR}/'")


# ─────────────────────────────────────────
# LỆNH QUIT
# ─────────────────────────────────────────

def ket_thuc_phien():
    """
    Gửi lệnh QUIT để kết thúc phiên làm việc và đóng kết nối.
    """
    global control_socket, server_ip

    if control_socket is None:
        print("[CLIENT] Chưa kết nối tới server nào.")
        return

    # Gửi lệnh QUIT
    gui_lenh("QUIT")
    phan_hoi = nhan_phan_hoi()  # "221 Goodbye"
    print(f"[CLIENT] {phan_hoi}")

    # Đóng socket
    control_socket.close()
    control_socket = None
    server_ip = None
    print("[CLIENT] Đã ngắt kết nối.")


# ─────────────────────────────────────────
# CHƯƠNG TRÌNH CHÍNH
# ─────────────────────────────────────────

def main():
    print("=" * 40)
    print("       FTP CLIENT - ĐỀ TÀI 13")
    print("=" * 40)
    print("Lệnh:")
    print("  login <IP> <username> <password>")
    print("  mget <tên_thư_mục>")
    print("  quit")
    print("-" * 40)

    while True:
        try:
            nhap = input("> ").strip()
        except KeyboardInterrupt:
            print("\n[CLIENT] Thoát chương trình.")
            break

        if not nhap:
            continue    # bỏ qua dòng trống

        # Tách lệnh và tham số
        phan = nhap.split()
        ten_lenh = phan[0].lower()

        # ── Lệnh login ──
        if ten_lenh == "login":
            if len(phan) != 4:
                print("Cách dùng: login <IP> <username> <password>")
                continue
            _, ip, ten_nd, mat_khau = phan
            dang_nhap(ip, ten_nd, mat_khau)

        # ── Lệnh mget ──
        elif ten_lenh == "mget":
            if len(phan) != 2:
                print("Cách dùng: mget <tên_thư_mục>")
                continue
            _, ten_thu_muc = phan
            tai_toan_bo_thu_muc(ten_thu_muc)

        # ── Lệnh quit ──
        elif ten_lenh == "quit":
            ket_thuc_phien()
            break

        else:
            print(f"Lệnh không nhận ra: '{ten_lenh}'. Dùng: login | mget | quit")


if __name__ == "__main__":
    main()

