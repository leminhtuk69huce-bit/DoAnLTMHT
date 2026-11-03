"""
client.py - Chương trình Client cho đề tài FTP v5 (Bulk Download)
=================================================================
Chức năng: Kết nối tới server FTP, đăng nhập, tải toàn bộ file
           trong một thư mục chỉ định về máy local.
Lệnh: login <IP> <user> <pass> | mget <thư_mục> | help | quit
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

os.makedirs(DOWNLOAD_DIR, exist_ok=True)


# ─────────────────────────────────────────
# HÀM TIỆN ÍCH: Gửi / Nhận qua socket
# ─────────────────────────────────────────

def gui_lenh(lenh):
    control_socket.sendall((lenh + "\r\n").encode("utf-8"))


def nhan_phan_hoi():
    du_lieu = b""
    while not du_lieu.endswith(b"\r\n"):
        byte = control_socket.recv(1)
        if not byte:
            break
        du_lieu += byte
    return du_lieu.decode("utf-8").strip()


def lay_so_cong(phan_hoi):
    phan = phan_hoi.split()
    return int(phan[1])


def ket_noi_data(so_cong):
    data_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    data_sock.connect((server_ip, so_cong))
    return data_sock


# ─────────────────────────────────────────
# HÀM TRỢ GIÚP (HELP)
# ─────────────────────────────────────────

def tro_giup_client():
    print("\n================= HƯỚNG DẪN LỆNH CLIENT ================")
    print("  login <IP> <user> <pass> : Kết nối và đăng nhập vào FTP Server")
    print("                             Ví dụ: login 127.0.0.1 admin 1234")
    print("                             (Hoặc IP LAN, vd: login 192.168.1.168 admin 1234)\n")
    print("  mget <tên_thư_mục>       : Tải toàn bộ file trong thư mục từ server")
    print("                             Ví dụ: mget docs")
    print("                             (Lưu ý: chỉ điền tên thư mục, không điền ổ đĩa E:\\...)\n")
    print("  help                     : Hiển thị danh sách câu lệnh này\n")
    print("  quit                     : Đóng kết nối và thoát chương trình")
    print("========================================================\n")


# ─────────────────────────────────────────
# LỆNH LOGIN
# ─────────────────────────────────────────

def dang_nhap(ip, ten_nd, mat_khau):
    global control_socket, server_ip

    try:
        control_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        control_socket.connect((ip, SERVER_PORT))
        server_ip = ip
        print(f"[CLIENT] Đã kết nối tới server {ip}:{SERVER_PORT}")
    except ConnectionRefusedError:
        print(f"[CLIENT] Không thể kết nối tới {ip}:{SERVER_PORT}. Server đã khởi động và gõ 'start' chưa?")
        control_socket = None
        return

    phan_hoi = nhan_phan_hoi()
    print(f"[CLIENT] {phan_hoi}")

    gui_lenh(f"USER {ten_nd}")
    phan_hoi = nhan_phan_hoi()
    print(f"[CLIENT] {phan_hoi}")

    gui_lenh(f"PASS {mat_khau}")
    phan_hoi = nhan_phan_hoi()
    print(f"[CLIENT] {phan_hoi}")

    if phan_hoi.startswith("230"):
        print("[CLIENT] Đăng nhập thành công!")
        print("         (Gõ tiếp: mget docs để tải toàn bộ file mẫu)")
    else:
        print("[CLIENT] Đăng nhập thất bại. Kiểm tra lại tài khoản/mật khẩu.")
        control_socket.close()
        control_socket = None
        server_ip = None


# ─────────────────────────────────────────
# LỆNH MGET – BƯỚC A: LẤY DANH SÁCH FILE
# ─────────────────────────────────────────

def lay_danh_sach_file(ten_thu_muc):
    gui_lenh(f"LIST {ten_thu_muc}")
    phan_hoi = nhan_phan_hoi()

    if phan_hoi.startswith("550"):
        print(f"[CLIENT] Lỗi: Thư mục '{ten_thu_muc}' không tồn tại trên server.")
        print("         (Gợi ý: Thư mục mẫu trên server là: docs)")
        return []

    so_cong = lay_so_cong(phan_hoi)

    data_sock = ket_noi_data(so_cong)
    du_lieu_nhan = b""
    while True:
        khoi = data_sock.recv(4096)
        if not khoi:
            break
        du_lieu_nhan += khoi
    data_sock.close()

    nhan_phan_hoi()

    ds_file = [f for f in du_lieu_nhan.decode("utf-8").split("\r\n") if f]
    return ds_file


# ─────────────────────────────────────────
# LỆNH MGET – BƯỚC B: TẢI MỘT FILE
# ─────────────────────────────────────────

def tai_mot_file(ten_thu_muc, ten_file, so_thu_tu, tong_so):
    gui_lenh(f"RETR {ten_thu_muc}/{ten_file}")
    phan_hoi = nhan_phan_hoi()

    if phan_hoi.startswith("550"):
        print(f"[CLIENT] Lỗi: Không thể tải file '{ten_file}'")
        return

    so_cong = lay_so_cong(phan_hoi)

    data_sock = ket_noi_data(so_cong)
    noi_dung_file = b""
    while True:
        khoi = data_sock.recv(4096)
        if not khoi:
            break
        noi_dung_file += khoi
    data_sock.close()

    nhan_phan_hoi()

    duong_dan_luu = os.path.join(DOWNLOAD_DIR, ten_file)
    with open(duong_dan_luu, "wb") as f:
        f.write(noi_dung_file)

    print(f"[CLIENT] Đã tải {so_thu_tu}/{tong_so} file: {ten_file}")


# ─────────────────────────────────────────
# LỆNH MGET – TỔNG HỢP
# ─────────────────────────────────────────

def tai_toan_bo_thu_muc(ten_thu_muc):
    if control_socket is None:
        print("[CLIENT] Chưa đăng nhập! Dùng lệnh: login <IP> admin 1234 (hoặc gõ 'help')")
        return

    print(f"[CLIENT] Đang lấy danh sách file trong thư mục '{ten_thu_muc}'...")
    ds_file = lay_danh_sach_file(ten_thu_muc)

    if not ds_file:
        print(f"[CLIENT] Thư mục '{ten_thu_muc}' rỗng hoặc không tồn tại.")
        return

    tong_so = len(ds_file)
    print(f"[CLIENT] Tìm thấy {tong_so} file trong thư mục '{ten_thu_muc}'")

    for thu_tu, ten_file in enumerate(ds_file, start=1):
        tai_mot_file(ten_thu_muc, ten_file, thu_tu, tong_so)

    print(f"[CLIENT] Hoàn thành! Đã tải {tong_so}/{tong_so} file"
          f" về thư mục '{DOWNLOAD_DIR}/'")


# ─────────────────────────────────────────
# LỆNH QUIT
# ─────────────────────────────────────────

def ket_thuc_phien():
    global control_socket, server_ip

    if control_socket is None:
        print("[CLIENT] Chưa kết nối tới server nào.")
        return

    gui_lenh("QUIT")
    phan_hoi = nhan_phan_hoi()
    print(f"[CLIENT] {phan_hoi}")

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
    print("(Gõ 'help' để xem danh sách lệnh)")
    print("-" * 40)

    while True:
        try:
            nhap = input("client> ").strip()
        except KeyboardInterrupt:
            print("\n[CLIENT] Thoát chương trình.")
            break

        if not nhap:
            continue

        phan = nhap.split()
        ten_lenh = phan[0].lower()

        if ten_lenh == "login":
            if len(phan) != 4:
                print("Cách dùng: login <IP> <username> <password>")
                print("Ví dụ   : login 127.0.0.1 admin 1234")
                continue
            _, ip, ten_nd, mat_khau = phan
            dang_nhap(ip, ten_nd, mat_khau)

        elif ten_lenh == "mget":
            if len(phan) != 2:
                print("Cách dùng: mget <tên_thư_mục>")
                print("Ví dụ   : mget docs")
                continue
            _, ten_thu_muc = phan
            tai_toan_bo_thu_muc(ten_thu_muc)

        elif ten_lenh == "help":
            tro_giup_client()

        elif ten_lenh == "quit":
            ket_thuc_phien()
            break

        else:
            print(f"Lệnh không hợp lệ: '{ten_lenh}'. Gõ 'help' để xem hướng dẫn.")


if __name__ == "__main__":
    main()
