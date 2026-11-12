#include "FTPClient.h"
#include <sstream>
#include <fstream>

using namespace std;
#pragma comment(lib, "ws2_32.lib")

SOCKET socketDieuKhien = INVALID_SOCKET;
string serverIP = "";

bool khoiTaoWinsock() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        return false;
    }
    CreateDirectoryA(THU_MUC_TAI, NULL);
    return true;
}

void donDepWinsock() {
    WSACleanup();
}

void troGiupClient() {
    cout << "\nDanh sach lenh Client:\n";
    cout << "  login <IP> <user> <pass> : Ket noi va dang nhap\n";
    cout << "  get <duong_dan_file>     : Truyen/Tai file tu Server\n";
    cout << "  help                     : Xem huong dan\n";
    cout << "  quit                     : Thoat chuong trinh\n\n";
}

void guiLenh(SOCKET s, const string& lenh) {
    string truyen = lenh + "\r\n";
    send(s, truyen.c_str(), (int)truyen.size(), 0);
}

string nhanPhanHoi(SOCKET s) {
    string phanHoi = "";
    char c;
    while (recv(s, &c, 1, 0) > 0) {
        if (c == '\r') continue;
        if (c == '\n') break;
        phanHoi += c;
    }
    if (!phanHoi.empty()) {
        cout << "[CLIENT] " << phanHoi << "\n";
    }
    return phanHoi;
}

int laySoCong(const string& phanHoi) {
    istringstream iss(phanHoi);
    string code;
    int cong = 0;
    iss >> code >> cong;
    return cong;
}

SOCKET ketNoiData(int soCong) {
    SOCKET dataSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (dataSocket == INVALID_SOCKET) {
        cout << "[CLIENT] Loi: Khong the tao socket data\n";
        return INVALID_SOCKET;
    }

    sockaddr_in diaChi;
    diaChi.sin_family = AF_INET;
    diaChi.sin_port = htons(soCong);
    diaChi.sin_addr.s_addr = inet_addr(serverIP.c_str());

    if (connect(dataSocket, (sockaddr*)&diaChi, sizeof(diaChi)) == SOCKET_ERROR) {
        cout << "[CLIENT] Loi: Khong the ket noi data toi cong " << soCong << "\n";
        closesocket(dataSocket);
        return INVALID_SOCKET;
    }

    return dataSocket;
}

void dangNhap(const string& ip, const string& tenND, const string& matKhau) {
    if (socketDieuKhien != INVALID_SOCKET) {
        cout << "[CLIENT] Da co ket noi. Vui long go 'quit' truoc khi ket noi moi.\n";
        return;
    }

    socketDieuKhien = socket(AF_INET, SOCK_STREAM, 0);
    if (socketDieuKhien == INVALID_SOCKET) {
        cout << "[CLIENT] Loi: Khong the tao socket\n";
        return;
    }

    sockaddr_in diaChiServer;
    diaChiServer.sin_family = AF_INET;
    diaChiServer.sin_port = htons(CONG_FTP);
    diaChiServer.sin_addr.s_addr = inet_addr(ip.c_str());

    cout << "[CLIENT] Dang ket noi toi " << ip << ":21 ...\n";
    if (connect(socketDieuKhien, (sockaddr*)&diaChiServer, sizeof(diaChiServer)) == SOCKET_ERROR) {
        cout << "[CLIENT] Loi: Ket noi that bai\n";
        closesocket(socketDieuKhien);
        socketDieuKhien = INVALID_SOCKET;
        return;
    }

    serverIP = ip;

    string phanHoi = nhanPhanHoi(socketDieuKhien);
    if (phanHoi.size() < 3 || phanHoi.substr(0, 3) != "220") {
        cout << "[CLIENT] Server khong san sang\n";
        closesocket(socketDieuKhien);
        socketDieuKhien = INVALID_SOCKET;
        return;
    }

    guiLenh(socketDieuKhien, "USER " + tenND);
    phanHoi = nhanPhanHoi(socketDieuKhien);

    guiLenh(socketDieuKhien, "PASS " + matKhau);
    phanHoi = nhanPhanHoi(socketDieuKhien);

    if (phanHoi.size() >= 3 && phanHoi.substr(0, 3) == "230") {
        cout << "[CLIENT] Dang nhap thanh cong!\n";
    } else {
        cout << "[CLIENT] Dang nhap that bai!\n";
        closesocket(socketDieuKhien);
        socketDieuKhien = INVALID_SOCKET;
    }
}

void taiFile(const string& duongDanFile) {
    if (socketDieuKhien == INVALID_SOCKET) {
        cout << "[CLIENT] Chua dang nhap! Dung lenh: login <IP> admin 1234\n";
        return;
    }

    if (duongDanFile.empty()) {
        cout << "[CLIENT] Vui long nhap ten file: get <ten_file>\n";
        return;
    }

    string tenFileLuu = duongDanFile;
    size_t viTriXuyet = tenFileLuu.find_last_of("/\\");
    if (viTriXuyet != string::npos) {
        tenFileLuu = tenFileLuu.substr(viTriXuyet + 1);
    }

    guiLenh(socketDieuKhien, "RETR " + duongDanFile);
    string phanHoi = nhanPhanHoi(socketDieuKhien);

    if (phanHoi.size() >= 3 && phanHoi.substr(0, 3) == "550") {
        cout << "[CLIENT] Loi: File khong ton tai tren server!\n";
        return;
    }

    int soCong = laySoCong(phanHoi);
    SOCKET dataSocket = ketNoiData(soCong);
    if (dataSocket == INVALID_SOCKET) return;

    string duongDanLuu = string(THU_MUC_TAI) + "\\" + tenFileLuu;
    ofstream file(duongDanLuu, ios::binary);
    if (!file.is_open()) {
        cout << "[CLIENT] Loi: Khong the tao file ghi\n";
        closesocket(dataSocket);
        return;
    }

    char buffer[4096];
    while (true) {
        int soByteNhan = recv(dataSocket, buffer, sizeof(buffer), 0);
        if (soByteNhan <= 0) break;
        file.write(buffer, soByteNhan);
    }
    file.close();

    closesocket(dataSocket);
    nhanPhanHoi(socketDieuKhien);

    cout << "[CLIENT] Da tai thanh cong: " << duongDanLuu << "\n";
}

void ketThucPhien() {
    if (socketDieuKhien != INVALID_SOCKET) {
        guiLenh(socketDieuKhien, "QUIT");
        nhanPhanHoi(socketDieuKhien);
        closesocket(socketDieuKhien);
        socketDieuKhien = INVALID_SOCKET;
    }
    cout << "[CLIENT] Phien lam viec da ket thuc.\n";
}
