#ifndef FTPSERVER_H
#define FTPSERVER_H

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>

#define CONG_FTP 21
#define THU_MUC_GOC "server_files"

bool khoiTaoWinsock();
void donDepWinsock();
void khoiDongServer();
void tatServer();
void troGiupServer();

void guiPhanHoi(SOCKET s, const std::string& thongBao);
std::string nhanLenh(SOCKET s);
SOCKET taoDataServer(int& cong);

bool xacThuc(SOCKET ctrl);
void xuLyRetr(SOCKET ctrl, const std::string& tenFile);
void xuLyClient(SOCKET ctrl);
void vongLapAccept();

#endif
