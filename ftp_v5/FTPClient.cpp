#include "FTPClient.h"
#include <sstream>
#include <fstream>

using namespace std;
#pragma comment(lib, "ws2_32.lib")

// ══════════════════════════════════════════
// BIEN TOAN CUC CUA CLIENT
// ══════════════════════════════════════════
SOCKET socketDieuKhien = INVALID_SOCKET;
string serverIP        = "";


// ══════════════════════════════════════════
// KHOI TAO & DON DEP WINSOCK
// ══════════════════════════════════════════

bool khoiTaoWinsock() {
    WSADATA wsa;
    cout << "Initialising Winsock...Initialised.\n\n";
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "Failed. Error Code : " << WSAGetLastError() << "\n";
        return false;
    }
    CreateDirectoryA(THU_MUC_TAI, NULL);
    return true;
}

void donDepWinsock() {
    WSACleanup();
}


// ══════════════════════════════════════════
// HAM TIEN ICH: GUI / NHAN QUA SOCKET
// ══════════════════════════════════════════

void guiLenh(SOCKET s, const string& lenh) {
    string truyen = lenh + "\r\n";
    send(s, truyen.c_str(), (int)truyen.size(), 0);
}

string nhanPhanHoi(SOCKET s) {
    string ket = "";
    char c;
    while (true) {
        int r = recv(s, &c, 1, 0);
        if (r <= 0) break;
        if (c == '\n') break;
        if (c != '\r') ket += c;
    }
    return ket;
}

int laySoCong(const string& phanHoi) {
    istringstream iss(phanHoi);
    string ma, cong;
    iss >> ma >> cong;
    return stoi(cong);
}

SOCKET ketNoiData(int soCong) {
    SOCKET ds = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in diaChiServer;
    diaChiServer.sin_family      = AF_INET;
    diaChiServer.sin_addr.s_addr = inet_addr(serverIP.c_str());
    diaChiServer.sin_port        = htons((u_short)soCong);

    if (connect(ds, (sockaddr*)&diaChiServer, sizeof(diaChiServer)) < 0) {
        cout << "[CLIENT] Loi ket noi data connection!\n";
        closesocket(ds);
        return INVALID_SOCKET;
    }
    return ds;
}


// ══════════════════════════════════════════
// HUONG DAN LENH (HELP)
// ══════════════════════════════════════════

void troGiupClient() {
    cout << "\n================= HUONG DAN LENH CLIENT ================\n";
    cout << "  login <IP> <user> <pass> : Ket noi va dang nhap vao FTP Server\n";
    cout << "                             Vi du: login 127.0.0.1 admin 1234\n";
    cout << "                             (Hoac dung IP LAN, vd: login 192.168.1.168 admin 1234)\n\n";
    cout << "  mget <ten_thu_muc>       : Tai toan bo file trong thu muc tu server\n";
    cout << "                             Vi du: mget docs\n";
    cout << "                             (Luu y: chi dien ten thu muc con, khong dien o dia E:\\...)\n\n";
    cout << "  help                     : Hien thi danh sach huong dan lenh nay\n\n";
    cout << "  quit                     : Ngat ket noi va thoat chuong trinh\n";
    cout << "========================================================\n\n";
}


// ══════════════════════════════════════════
// LENH LOGIN
// ══════════════════════════════════════════

void dangNhap(const string& ip, const string& tenND, const string& matKhau) {
    socketDieuKhien = socket(AF_INET, SOCK_STREAM, 0);
    if (socketDieuKhien == INVALID_SOCKET) {
        cout << "[CLIENT] Loi tao socket: " << WSAGetLastError() << "\n";
        return;
    }
    cout << "Socket created.\n";

    sockaddr_in serverAddr;
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    serverAddr.sin_port        = htons(CONG_FTP);

    if (connect(socketDieuKhien, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cout << "[CLIENT] Khong the ket noi toi " << ip << ":" << CONG_FTP << "\n";
        cout << "         (Hay chac chan FTPServer da chay va da go lenh 'start')\n";
        closesocket(socketDieuKhien);
        socketDieuKhien = INVALID_SOCKET;
        return;
    }
    cout << "Connected.\n";
    serverIP = ip;

    string phanHoi = nhanPhanHoi(socketDieuKhien);
    cout << "[CLIENT] " << phanHoi << "\n";

    guiLenh(socketDieuKhien, "USER " + tenND);
    phanHoi = nhanPhanHoi(socketDieuKhien);
    cout << "[CLIENT] " << phanHoi << "\n";

    guiLenh(socketDieuKhien, "PASS " + matKhau);
    phanHoi = nhanPhanHoi(socketDieuKhien);
    cout << "[CLIENT] " << phanHoi << "\n";

    if (phanHoi.size() >= 3 && phanHoi.substr(0, 3) == "230") {
        cout << "[CLIENT] Dang nhap thanh cong!\n";
        cout << "         (Ban co the go tiep: mget docs de tai thu muc)\n";
    } else {
        cout << "[CLIENT] Dang nhap that bai! Kiem tra lai tai khoan/mat khau.\n";
        closesocket(socketDieuKhien);
        socketDieuKhien = INVALID_SOCKET;
        serverIP = "";
    }
}


// ══════════════════════════════════════════
// MGET BUOC A: LAY DANH SACH FILE
// ══════════════════════════════════════════

vector<string> layDanhSachFile(const string& tenThuMuc) {
    vector<string> dsFile;

    guiLenh(socketDieuKhien, "LIST " + tenThuMuc);
    string phanHoi = nhanPhanHoi(socketDieuKhien);

    if (phanHoi.size() >= 3 && phanHoi.substr(0, 3) == "550") {
        cout << "[CLIENT] Loi: Thu muc '" << tenThuMuc << "' khong ton tai tren server!\n";
        cout << "         (Goi y: Thu muc mau tren server la: docs)\n";
        return dsFile;
    }

    int soCong = laySoCong(phanHoi);

    SOCKET dataSocket = ketNoiData(soCong);
    if (dataSocket == INVALID_SOCKET) return dsFile;

    string tenFile = "";
    char c;
    while (true) {
        int r = recv(dataSocket, &c, 1, 0);
        if (r <= 0) {
            if (!tenFile.empty()) dsFile.push_back(tenFile);
            break;
        }
        if (c == '\n') {
            if (!tenFile.empty()) {
                dsFile.push_back(tenFile);
                tenFile = "";
            }
        } else if (c != '\r') {
            tenFile += c;
        }
    }
    closesocket(dataSocket);

    nhanPhanHoi(socketDieuKhien);
    return dsFile;
}


// ══════════════════════════════════════════
// MGET BUOC B: TAI MOT FILE
// ══════════════════════════════════════════

void taiMotFile(const string& tenThuMuc, const string& tenFile, int soThuTu, int tongSo) {
    guiLenh(socketDieuKhien, "RETR " + tenThuMuc + "/" + tenFile);
    string phanHoi = nhanPhanHoi(socketDieuKhien);

    if (phanHoi.size() >= 3 && phanHoi.substr(0, 3) == "550") {
        cout << "[CLIENT] Loi: Khong the tai file '" << tenFile << "'\n";
        return;
    }

    int soCong = laySoCong(phanHoi);

    SOCKET dataSocket = ketNoiData(soCong);
    if (dataSocket == INVALID_SOCKET) return;

    string duongDanLuu = string(THU_MUC_TAI) + "\\" + tenFile;
    ofstream file(duongDanLuu, ios::binary);
    if (!file.is_open()) {
        cout << "[CLIENT] Loi: Khong the tao file '" << duongDanLuu << "'\n";
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

    cout << "[CLIENT] Da tai " << soThuTu << "/" << tongSo << " file: " << tenFile << "\n";
}


// ══════════════════════════════════════════
// LENH MGET TONG HOP
// ══════════════════════════════════════════

void taiToanBoThuMuc(const string& tenThuMuc) {
    if (socketDieuKhien == INVALID_SOCKET) {
        cout << "[CLIENT] Chua dang nhap! Go 'help' hoac dung lenh: login <IP> admin 1234\n";
        return;
    }

    cout << "[CLIENT] Dang lay danh sach file trong thu muc '" << tenThuMuc << "'...\n";
    vector<string> dsFile = layDanhSachFile(tenThuMuc);

    if (dsFile.empty()) {
        cout << "[CLIENT] Thu muc '" << tenThuMuc << "' trong hoac khong ton tai.\n";
        return;
    }

    int tongSo = (int)dsFile.size();
    cout << "[CLIENT] Tim thay " << tongSo << " file trong thu muc '" << tenThuMuc << "'\n";

    for (int i = 0; i < tongSo; i++) {
        taiMotFile(tenThuMuc, dsFile[i], i + 1, tongSo);
    }

    cout << "[CLIENT] Hoan thanh! Da tai " << tongSo << "/" << tongSo
         << " file ve thu muc '" << THU_MUC_TAI << "'\n";
}


// ══════════════════════════════════════════
// LENH QUIT
// ══════════════════════════════════════════

void ketThucPhien() {
    if (socketDieuKhien == INVALID_SOCKET) {
        cout << "[CLIENT] Chua ket noi toi server nao.\n";
        return;
    }

    guiLenh(socketDieuKhien, "QUIT");
    string phanHoi = nhanPhanHoi(socketDieuKhien);
    cout << "[CLIENT] " << phanHoi << "\n";

    closesocket(socketDieuKhien);
    socketDieuKhien = INVALID_SOCKET;
    serverIP = "";
    cout << "[CLIENT] Da ngat ket noi.\n";
}
