#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

// ══════════════════════════════════════════
// CAI DAT CHUNG
// ══════════════════════════════════════════
#define CONG_FTP     21                   // Cong dieu khien FTP chuan
#define THU_MUC_TAI  "client_downloads"  // Thu muc luu file tai ve

// ══════════════════════════════════════════
// KHAI BAO CAC HAM CUA CLIENT
// ══════════════════════════════════════════

// Khoi tao va don dep Winsock
bool khoiTaoWinsock();
void donDepWinsock();

// Cac chuc nang chinh theo yeu cau de bai
void troGiupClient();
void dangNhap(const std::string& ip, const std::string& tenND, const std::string& matKhau);
void taiToanBoThuMuc(const std::string& tenThuMuc);
void ketThucPhien();

// Cac ham ho tro giao thuc FTP
void guiLenh(SOCKET s, const std::string& lenh);
std::string nhanPhanHoi(SOCKET s);
int laySoCong(const std::string& phanHoi);
SOCKET ketNoiData(int soCong);
std::vector<std::string> layDanhSachFile(const std::string& tenThuMuc);
void taiMotFile(const std::string& tenThuMuc, const std::string& tenFile, int soThuTu, int tongSo);

#endif // FTPCLIENT_H

