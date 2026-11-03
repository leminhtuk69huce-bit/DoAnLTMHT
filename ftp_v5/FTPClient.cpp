/*
 * FTPClient.cpp - Chuong trinh Client FTP v5 (Bulk Download) - De tai 13
 * =========================================================================
 * Bien dich: CodeBlocks, them "-lws2_32" vao Linker Options
 *
 * Lenh su dung:
 *   login <IP server> <username> <password>
 *   mget <ten thu muc tren server>
 *   quit
 */

#include <winsock2.h>    // Thu vien socket chinh (giong code mau cua thay)
#include <windows.h>     // Thu vien Windows (de tao thu muc: CreateDirectoryA)
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;
#pragma comment(lib, "ws2_32.lib")  // Lien ket thu vien Winsock (giong code thay)


// ══════════════════════════════════════════
// CAI DAT CHUNG
// ══════════════════════════════════════════

#define CONG_FTP     21                    // Cong dieu khien FTP
#define THU_MUC_TAI  "client_downloads"   // Thu muc luu file tai ve


// ══════════════════════════════════════════
// BIEN TOAN CUC
// ══════════════════════════════════════════

SOCKET socketDieuKhien = INVALID_SOCKET;  // Socket ket noi dieu khien (cong 21)
string serverIP        = "";              // IP cua server dang ket noi


// ══════════════════════════════════════════
// HAM TIEN ICH: GUI / NHAN QUA SOCKET
// ══════════════════════════════════════════

// Gui lenh toi server qua Control Connection.
// FTP yeu cau moi lenh ket thuc bang \r\n
void guiLenh(SOCKET s, const string& lenh) {
    string truyen = lenh + "\r\n";
    send(s, truyen.c_str(), (int)truyen.size(), 0);
}

// Nhan mot dong phan hoi tu server (doc den khi gap \n)
string nhanPhanHoi(SOCKET s) {
    string ket = "";
    char c;
    while (true) {
        int r = recv(s, &c, 1, 0);
        if (r <= 0) break;       // Ket noi bi dut
        if (c == '\n') break;    // Het dong phan hoi
        if (c != '\r') ket += c; // Bo \r, giu lai noi dung
    }
    return ket;
}

// Trich xuat so cong tu chuoi phan hoi "150 <CONG> Opening..."
// Vi du: "150 54321 Opening..." -> tra ve 54321
int laySoCong(const string& phanHoi) {
    // Tach chuoi theo khoang trang: ["150", "54321", "Opening", ...]
    istringstream iss(phanHoi);
    string ma, cong;
    iss >> ma >> cong;   // Doc "150" vao ma, "54321" vao cong
    return stoi(cong);   // Chuyen "54321" thanh so nguyen 54321
}

// Tao Data Connection moi toi server tai cong chi dinh
SOCKET ketNoiData(int soCong) {
    SOCKET ds = socket(AF_INET, SOCK_STREAM, 0);  // Tao socket TCP moi

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
// LENH LOGIN: Ket noi va dang nhap
// ══════════════════════════════════════════

void dangNhap(const string& ip, const string& tenND, const string& matKhau) {
    // Buoc 1: Tao socket va ket noi TCP toi server cong 21
    // (y chang cach lam trong code mau cua thay, muc 1.3 va 1.4)
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
        cout << "         (Server da chay chua? Chay FTPServer.exe truoc)\n";
        closesocket(socketDieuKhien);
        socketDieuKhien = INVALID_SOCKET;
        return;
    }
    cout << "Connected.\n";
    serverIP = ip;

    // Buoc 2: Nhan thong bao chao mung tu server (220 FTP Server Ready)
    string phanHoi = nhanPhanHoi(socketDieuKhien);
    cout << "[CLIENT] " << phanHoi << "\n";

    // Buoc 3: Gui ten dang nhap
    guiLenh(socketDieuKhien, "USER " + tenND);
    phanHoi = nhanPhanHoi(socketDieuKhien);
    cout << "[CLIENT] " << phanHoi << "\n";  // In: "331 Password required"

    // Buoc 4: Gui mat khau
    guiLenh(socketDieuKhien, "PASS " + matKhau);
    phanHoi = nhanPhanHoi(socketDieuKhien);
    cout << "[CLIENT] " << phanHoi << "\n";  // In: "230 Login successful" hoac "530..."

    // Buoc 5: Kiem tra ket qua
    if (phanHoi.size() >= 3 && phanHoi.substr(0, 3) == "230") {
        cout << "[CLIENT] Dang nhap thanh cong!\n";
    } else {
        cout << "[CLIENT] Dang nhap that bai! Kiem tra lai tai khoan/mat khau.\n";
        closesocket(socketDieuKhien);
        socketDieuKhien = INVALID_SOCKET;
        serverIP = "";
    }
}


// ══════════════════════════════════════════
// MGET BUOC A: Lay danh sach file tu server
// ══════════════════════════════════════════

vector<string> layDanhSachFile(const string& tenThuMuc) {
    vector<string> dsFile;

    // Gui lenh LIST qua Control Connection
    guiLenh(socketDieuKhien, "LIST " + tenThuMuc);
    string phanHoi = nhanPhanHoi(socketDieuKhien);

    // Neu server bao loi 550 (thu muc khong ton tai)
    if (phanHoi.size() >= 3 && phanHoi.substr(0, 3) == "550") {
        cout << "[CLIENT] Loi: Thu muc '" << tenThuMuc << "' khong ton tai tren server!\n";
        return dsFile;  // Tra ve danh sach rong
    }

    // Lay so cong Data Connection tu "150 <CONG> ..."
    int soCong = laySoCong(phanHoi);

    // Ket noi vao Data Connection de nhan danh sach file
    SOCKET dataSocket = ketNoiData(soCong);
    if (dataSocket == INVALID_SOCKET) return dsFile;

    // Nhan du lieu: moi dong la mot ten file, ket thuc bang \r\n
    // Doc tung ki tu, gom thanh tung ten file
    string tenFile = "";
    char c;
    while (true) {
        int r = recv(dataSocket, &c, 1, 0);
        if (r <= 0) {
            // Server dong ket noi → het du lieu
            if (!tenFile.empty()) dsFile.push_back(tenFile);
            break;
        }
        if (c == '\n') {
            // Ket thuc mot dong = mot ten file
            if (!tenFile.empty()) {
                dsFile.push_back(tenFile);
                tenFile = "";  // Reset de doc ten file tiep theo
            }
        } else if (c != '\r') {
            tenFile += c;  // Gom tung ki tu vao ten file
        }
    }
    closesocket(dataSocket);

    // Nhan "226 Transfer complete" tu Control Connection
    nhanPhanHoi(socketDieuKhien);

    return dsFile;
}


// ══════════════════════════════════════════
// MGET BUOC B: Tai mot file cu the
// ══════════════════════════════════════════

void taiMotFile(const string& tenThuMuc, const string& tenFile, int soThuTu, int tongSo) {
    // Gui lenh RETR qua Control Connection
    // Vi du: "RETR docs/file1.txt"
    guiLenh(socketDieuKhien, "RETR " + tenThuMuc + "/" + tenFile);
    string phanHoi = nhanPhanHoi(socketDieuKhien);

    if (phanHoi.size() >= 3 && phanHoi.substr(0, 3) == "550") {
        cout << "[CLIENT] Loi: Khong the tai file '" << tenFile << "'\n";
        return;
    }

    // Lay so cong Data Connection
    int soCong = laySoCong(phanHoi);

    // Ket noi Data Connection de nhan noi dung file
    SOCKET dataSocket = ketNoiData(soCong);
    if (dataSocket == INVALID_SOCKET) return;

    // Mo file de ghi (che do nhi phan: ios::binary - de tai duoc ca file anh, zip...)
    string duongDanLuu = string(THU_MUC_TAI) + "\\" + tenFile;
    ofstream file(duongDanLuu, ios::binary);
    if (!file.is_open()) {
        cout << "[CLIENT] Loi: Khong the tao file '" << duongDanLuu << "'\n";
        closesocket(dataSocket);
        return;
    }

    // Nhan noi dung file theo tung khoi 4KB va ghi xuong dia
    char buffer[4096];
    while (true) {
        int soByteNhan = recv(dataSocket, buffer, sizeof(buffer), 0);
        if (soByteNhan <= 0) break;  // Server dong data → het file
        file.write(buffer, soByteNhan);
    }
    file.close();

    closesocket(dataSocket);

    // Nhan "226 Transfer complete" tu Control Connection
    nhanPhanHoi(socketDieuKhien);

    // Hien thi tien trinh tai file
    cout << "[CLIENT] Da tai " << soThuTu << "/" << tongSo << " file: " << tenFile << "\n";
}


// ══════════════════════════════════════════
// LENH MGET: Tai toan bo thu muc
// ══════════════════════════════════════════

void taiToanBoThuMuc(const string& tenThuMuc) {
    // Kiem tra da dang nhap chua
    if (socketDieuKhien == INVALID_SOCKET) {
        cout << "[CLIENT] Chua dang nhap! Dung lenh: login <IP> <username> <password>\n";
        return;
    }

    // === BUOC A: Lay danh sach file ===
    cout << "[CLIENT] Dang lay danh sach file trong thu muc '" << tenThuMuc << "'...\n";
    vector<string> dsFile = layDanhSachFile(tenThuMuc);

    if (dsFile.empty()) {
        cout << "[CLIENT] Thu muc '" << tenThuMuc << "' trong hoac khong ton tai.\n";
        return;
    }

    int tongSo = (int)dsFile.size();
    cout << "[CLIENT] Tim thay " << tongSo << " file trong thu muc '" << tenThuMuc << "'\n";

    // === BUOC B: Tai tung file (lap qua danh sach) ===
    for (int i = 0; i < tongSo; i++) {
        taiMotFile(tenThuMuc, dsFile[i], i + 1, tongSo);
    }

    cout << "[CLIENT] Hoan thanh! Da tai " << tongSo << "/" << tongSo
         << " file ve thu muc '" << THU_MUC_TAI << "'\n";
}


// ══════════════════════════════════════════
// LENH QUIT: Ket thuc phien lam viec
// ══════════════════════════════════════════

void ketThucPhien() {
    if (socketDieuKhien == INVALID_SOCKET) {
        cout << "[CLIENT] Chua ket noi toi server nao.\n";
        return;
    }

    // Gui lenh QUIT va nhan phan hoi 221
    guiLenh(socketDieuKhien, "QUIT");
    string phanHoi = nhanPhanHoi(socketDieuKhien);
    cout << "[CLIENT] " << phanHoi << "\n";  // In: "221 Goodbye"

    // Dong socket
    closesocket(socketDieuKhien);
    socketDieuKhien = INVALID_SOCKET;
    serverIP = "";
    cout << "[CLIENT] Da ngat ket noi.\n";
}


// ══════════════════════════════════════════
// CHUONG TRINH CHINH
// ══════════════════════════════════════════

int main() {
    // --- Khoi tao Winsock (copy y chang code mau cua thay) ---
    WSADATA wsa;
    cout << "\nInitialising Winsock...";
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "Failed. Error Code : " << WSAGetLastError() << "\n";
        return 1;
    }
    cout << "Initialised.\n\n";

    // Tao thu muc luu file neu chua co
    CreateDirectoryA(THU_MUC_TAI, NULL);

    cout << "========================================\n";
    cout << "      FTP CLIENT - DE TAI 13\n";
    cout << "========================================\n";
    cout << "Lenh:\n";
    cout << "  login <IP> <username> <password>\n";
    cout << "  mget <ten_thu_muc>\n";
    cout << "  quit\n";
    cout << "----------------------------------------\n";

    string dong;
    while (true) {
        cout << "client> ";
        getline(cin, dong);

        if (dong.empty()) continue;  // Bo qua dong trong

        // Tach lenh va tham so (theo khoang trang)
        istringstream iss(dong);
        vector<string> phan;
        string tu;
        while (iss >> tu) phan.push_back(tu);

        if (phan.empty()) continue;
        string tenLenh = phan[0];

        // --- Lenh login ---
        if (tenLenh == "login") {
            if (phan.size() != 4) {
                cout << "Cach dung: login <IP> <username> <password>\n";
                continue;
            }
            dangNhap(phan[1], phan[2], phan[3]);

        // --- Lenh mget ---
        } else if (tenLenh == "mget") {
            if (phan.size() != 2) {
                cout << "Cach dung: mget <ten_thu_muc>\n";
                continue;
            }
            taiToanBoThuMuc(phan[1]);

        // --- Lenh quit ---
        } else if (tenLenh == "quit") {
            ketThucPhien();
            break;

        } else {
            cout << "Lenh khong duoc ho tro: '" << tenLenh << "'\n";
            cout << "Go: login | mget | quit\n";
        }
    }

    WSACleanup();  // Don dep Winsock truoc khi thoat (giong code mau thay)
    return 0;
}

