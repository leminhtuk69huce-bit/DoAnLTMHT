#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>

#define CONG_FTP 21
#define THU_MUC_TAI "client_downloads"

bool khoiTaoWinsock();
void donDepWinsock();
void troGiupClient();

void dangNhap(const std::string& ip, const std::string& tenND, const std::string& matKhau);
void taiFile(const std::string& duongDanFile);
void ketThucPhien();

void guiLenh(SOCKET s, const std::string& lenh);
std::string nhanPhanHoi(SOCKET s);
int laySoCong(const std::string& phanHoi);
SOCKET ketNoiData(int soCong);

#endif
