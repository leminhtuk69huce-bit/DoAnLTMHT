#include "FTPServer.h"
#include <sstream>
#include <fstream>
#include <thread>

using namespace std;
#pragma comment(lib, "ws2_32.lib")

// ══════════════════════════════════════════
// BIEN TOAN CUC CUA SERVER
// ══════════════════════════════════════════
const string TAI_KHOAN = "admin";
const string MAT_KHAU  = "1234";

SOCKET socketChinh = INVALID_SOCKET;
bool   dangChay    = false;


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
    return true;
}

void donDepWinsock() {
    WSACleanup();
}


// ══════════════════════════════════════════
// HAM TIEN ICH: GUI / NHAN QUA SOCKET
// ══════════════════════════════════════════

void guiPhanHoi(SOCKET s, const string& thongBao) {
    string truyen = thongBao + "\r\n";
    send(s, truyen.c_str(), (int)truyen.size(), 0);
}

string nhanLenh(SOCKET s) {
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


// ══════════════════════════════════════════
// HIEN THI IP & HUONG DAN (HELP)
// ══════════════════════════════════════════

void hienThiIPServer() {
    cout << "[SERVER] Dia chi IP de Client ket noi:\n";
    cout << "  - Cung may (Localhost) : 127.0.0.1\n";

    char tenMay[256];
    if (gethostname(tenMay, sizeof(tenMay)) == 0) {
        struct hostent* he = gethostbyname(tenMay);
        if (he != NULL) {
            for (int i = 0; he->h_addr_list[i] != NULL; ++i) {
                struct in_addr addr;
                memcpy(&addr, he->h_addr_list[i], sizeof(struct in_addr));
                string ipStr = inet_ntoa(addr);
                if (ipStr != "127.0.0.1") {
                    cout << "  - Mang LAN (May khac)  : " << ipStr << "\n";
                }
            }
        }
    }
    cout << "  => Cu phap ben Client : login <IP> admin 1234\n";
}

void troGiupServer() {
    cout << "\n================= HUONG DAN LENH SERVER ================\n";
    cout << "  start : Khoi dong server va lang nghe tren cong " << CONG_FTP << "\n";
    cout << "  stop  : Tam dung server\n";
    cout << "  ip    : Hien thi lai dia chi IP cua server\n";
    cout << "  help  : Hien thi danh sach huong dan lenh\n";
    cout << "  exit  : Tat server va thoat chuong trinh\n";
    cout << "========================================================\n\n";
}


// ══════════════════════════════════════════
// TAO DATA CONNECTION
// ══════════════════════════════════════════

SOCKET taoDataServer(int& cong) {
    SOCKET ds = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in diaChi;
    diaChi.sin_family      = AF_INET;
    diaChi.sin_addr.s_addr = INADDR_ANY;
    diaChi.sin_port        = htons(0);   // Port 0: he dieu hanh tu cap cong trong

    bind(ds, (sockaddr*)&diaChi, sizeof(diaChi));

    sockaddr_in diaChiBound;
    int len = sizeof(diaChiBound);
    getsockname(ds, (sockaddr*)&diaChiBound, &len);
    cong = ntohs(diaChiBound.sin_port);

    listen(ds, 1);
    return ds;
}


// ══════════════════════════════════════════
// DOC DANH SACH FILE
// ══════════════════════════════════════════

vector<string> layDanhSachFile(const string& tenThuMuc) {
    vector<string> dsFile;
    string pattern = string(THU_MUC_GOC) + "\\" + tenThuMuc + "\\*";

    WIN32_FIND_DATAA timKiem;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &timKiem);

    if (hFind == INVALID_HANDLE_VALUE) {
        return dsFile;
    }

    do {
        bool laThuMucCon = (timKiem.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        bool laDauCham   = (string(timKiem.cFileName) == "." || string(timKiem.cFileName) == "..");

        if (!laThuMucCon && !laDauCham) {
            dsFile.push_back(timKiem.cFileName);
        }
    } while (FindNextFileA(hFind, &timKiem));

    FindClose(hFind);
    return dsFile;
}


// ══════════════════════════════════════════
// XU LY XAC THUC (USER & PASS)
// ══════════════════════════════════════════

bool xacThuc(SOCKET ctrl) {
    string lenh = nhanLenh(ctrl);
    cout << "[SERVER] Nhan: " << lenh << "\n";

    if (lenh.size() < 6 || lenh.substr(0, 5) != "USER ") {
        guiPhanHoi(ctrl, "500 Lenh khong hop le");
        return false;
    }
    string tenND = lenh.substr(5);
    guiPhanHoi(ctrl, "331 Password required");

    lenh = nhanLenh(ctrl);
    cout << "[SERVER] Nhan: PASS ****\n";

    if (lenh.size() < 6 || lenh.substr(0, 5) != "PASS ") {
        guiPhanHoi(ctrl, "500 Lenh khong hop le");
        return false;
    }
    string matKhauNhap = lenh.substr(5);

    if (tenND == TAI_KHOAN && matKhauNhap == MAT_KHAU) {
        guiPhanHoi(ctrl, "230 Login successful");
        cout << "[SERVER] Xac thuc thanh cong cho user '" << tenND << "'\n";
        return true;
    } else {
        guiPhanHoi(ctrl, "530 Login incorrect");
        cout << "[SERVER] Xac thuc that bai cho user '" << tenND << "'\n";
        return false;
    }
}


// ══════════════════════════════════════════
// XU LY LENH LIST
// ══════════════════════════════════════════

void xuLyList(SOCKET ctrl, const string& tenThuMuc) {
    string duongDanThuMuc = string(THU_MUC_GOC) + "\\" + tenThuMuc;
    DWORD thuocTinh = GetFileAttributesA(duongDanThuMuc.c_str());
    bool tonTai = (thuocTinh != INVALID_FILE_ATTRIBUTES) &&
                  (thuocTinh & FILE_ATTRIBUTE_DIRECTORY);

    if (!tonTai) {
        guiPhanHoi(ctrl, "550 Directory not found");
        cout << "[SERVER] Thu muc '" << tenThuMuc << "' khong ton tai\n";
        return;
    }

    vector<string> dsFile = layDanhSachFile(tenThuMuc);
    cout << "[SERVER] Thu muc '" << tenThuMuc << "' co " << dsFile.size() << " file\n";

    int cong;
    SOCKET ds = taoDataServer(cong);
    guiPhanHoi(ctrl, "150 " + to_string(cong) + " Opening data connection");

    sockaddr_in diaChiClient;
    int lenClient = sizeof(diaChiClient);
    SOCKET dataConn = accept(ds, (sockaddr*)&diaChiClient, &lenClient);

    for (const string& tenFile : dsFile) {
        string dong = tenFile + "\r\n";
        send(dataConn, dong.c_str(), (int)dong.size(), 0);
    }

    closesocket(dataConn);
    closesocket(ds);

    guiPhanHoi(ctrl, "226 Transfer complete");
    cout << "[SERVER] Da gui xong danh sach file\n";
}


// ══════════════════════════════════════════
// XU LY LENH RETR
// ══════════════════════════════════════════

void xuLyRetr(SOCKET ctrl, const string& duongDanFile) {
    string duongDanChuan = duongDanFile;
    for (char& c : duongDanChuan) {
        if (c == '/') c = '\\';
    }

    string duongDanDayDu = string(THU_MUC_GOC) + "\\" + duongDanChuan;

    ifstream file(duongDanDayDu, ios::binary);
    if (!file.is_open()) {
        guiPhanHoi(ctrl, "550 File not found");
        cout << "[SERVER] File '" << duongDanFile << "' khong ton tai\n";
        return;
    }

    cout << "[SERVER] Dang gui file: " << duongDanFile << "\n";

    int cong;
    SOCKET ds = taoDataServer(cong);
    guiPhanHoi(ctrl, "150 " + to_string(cong) + " Opening data connection");

    sockaddr_in diaChiClient;
    int lenClient = sizeof(diaChiClient);
    SOCKET dataConn = accept(ds, (sockaddr*)&diaChiClient, &lenClient);

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        int soByte = (int)file.gcount();
        send(dataConn, buffer, soByte, 0);
    }
    file.close();

    closesocket(dataConn);
    closesocket(ds);

    guiPhanHoi(ctrl, "226 Transfer complete");
    cout << "[SERVER] Da gui xong file: " << duongDanFile << "\n";
}


// ══════════════════════════════════════════
// XU LY CLIENT (THREAD)
// ══════════════════════════════════════════

void xuLyClient(SOCKET ctrl) {
    guiPhanHoi(ctrl, "220 FTP Server Ready");

    if (!xacThuc(ctrl)) {
        closesocket(ctrl);
        return;
    }

    while (true) {
        string lenh = nhanLenh(ctrl);
        if (lenh.empty()) break;

        cout << "[SERVER] Nhan lenh: " << lenh << "\n";

        if (lenh.size() > 5 && lenh.substr(0, 5) == "LIST ") {
            string tenThuMuc = lenh.substr(5);
            xuLyList(ctrl, tenThuMuc);

        } else if (lenh.size() > 5 && lenh.substr(0, 5) == "RETR ") {
            string duongDan = lenh.substr(5);
            xuLyRetr(ctrl, duongDan);

        } else if (lenh == "QUIT") {
            guiPhanHoi(ctrl, "221 Goodbye");
            cout << "[SERVER] Client da ngat ket noi\n";
            break;

        } else {
            guiPhanHoi(ctrl, "500 Unknown command");
        }
    }

    closesocket(ctrl);
}


// ══════════════════════════════════════════
// VONG LAP ACCEPT
// ══════════════════════════════════════════

void vongLapAccept() {
    while (dangChay) {
        sockaddr_in diaChiClient;
        int lenClient = sizeof(diaChiClient);

        SOCKET clientSocket = accept(socketChinh, (sockaddr*)&diaChiClient, &lenClient);
        if (clientSocket == INVALID_SOCKET) break;

        cout << "[SERVER] Client " << inet_ntoa(diaChiClient.sin_addr) << " da ket noi\n";

        thread t(xuLyClient, clientSocket);
        t.detach();
    }
}


// ══════════════════════════════════════════
// KHOI DONG & TAT SERVER
// ══════════════════════════════════════════

void khoiDongServer() {
    if (dangChay) {
        cout << "[SERVER] Server da dang chay roi!\n";
        return;
    }

    socketChinh = socket(AF_INET, SOCK_STREAM, 0);
    if (socketChinh == INVALID_SOCKET) {
        cout << "[SERVER] Loi tao socket: " << WSAGetLastError() << "\n";
        return;
    }
    cout << "Socket created.\n";

    int opt = 1;
    setsockopt(socketChinh, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in serverAddr;
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port        = htons(CONG_FTP);

    if (bind(socketChinh, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "[SERVER] Loi bind cong " << CONG_FTP << ": " << WSAGetLastError() << "\n";
        cout << "         Giai phap: Chay ung dung bang 'Run as administrator'\n";
        closesocket(socketChinh);
        socketChinh = INVALID_SOCKET;
        return;
    }
    cout << "Bind done.\n";

    listen(socketChinh, 5);

    dangChay = true;
    cout << "[SERVER] Dang lang nghe tren cong " << CONG_FTP << "...\n";

    hienThiIPServer();

    thread t(vongLapAccept);
    t.detach();
}

void tatServer() {
    if (!dangChay) {
        cout << "[SERVER] Server chua chay!\n";
        return;
    }
    dangChay = false;
    closesocket(socketChinh);
    socketChinh = INVALID_SOCKET;
    cout << "[SERVER] Da tat server.\n";
}
