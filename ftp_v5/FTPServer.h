#ifndef FTPSERVER_H
#define FTPSERVER_H

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

// ══════════════════════════════════════════
// CAI DAT CHUNG
// ══════════════════════════════════════════
#define CONG_FTP     21              // Cong dieu khien FTP chuan
#define THU_MUC_GOC  "server_files"  // Thu muc goc chua file cua server

extern const std::string TAI_KHOAN;  // "admin"
extern const std::string MAT_KHAU;   // "1234"

// ══════════════════════════════════════════
// KHAI BAO CAC HAM CUA SERVER
// ══════════════════════════════════════════

// Khoi tao va don dep Winsock
bool khoiTaoWinsock();
void donDepWinsock();

// Dieu khien Server
void khoiDongServer();
void tatServer();
void hienThiIPServer();
void troGiupServer();

// Giao tiep Socket
void guiPhanHoi(SOCKET s, const std::string& thongBao);
std::string nhanLenh(SOCKET s);
SOCKET taoDataServer(int& cong);

// Xu ly giao thuc FTP
std::vector<std::string> layDanhSachFile(const std::string& tenThuMuc);
bool xacThuc(SOCKET ctrl);
void xuLyList(SOCKET ctrl, const std::string& tenThuMuc);
void xuLyRetr(SOCKET ctrl, const std::string& duongDanFile);
void xuLyClient(SOCKET ctrl);
void vongLapAccept();

#endif // FTPSERVER_H
