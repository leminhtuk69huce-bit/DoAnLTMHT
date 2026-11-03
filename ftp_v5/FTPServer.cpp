/*
 * FTPServer.cpp - Chuong trinh Server FTP v5 (Bulk Download) - De tai 13
 * =========================================================================
 * Bien dich: CodeBlocks, them "-lws2_32" vao Linker Options
 * Luu y: Cong 21 can quyen Administrator tren Windows.
 *        Click phai vao CodeBlocks -> "Run as administrator"
 *
 * Lenh dieu khien: start | stop | exit
 */

#include <winsock2.h>    // Thu vien socket chinh (giong code mau cua thay)
#include <windows.h>     // Thu vien Windows (de doc thu muc: FindFirstFile...)
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <thread>        // Xu ly da luong (nhieu client cung luc)

using namespace std;
#pragma comment(lib, "ws2_32.lib")  // Lien ket thu vien Winsock (giong code thay)


// ══════════════════════════════════════════
// CAI DAT CHUNG
// ══════════════════════════════════════════

#define CONG_FTP     21             // Cong FTP chuan (cung co the doi thanh 2121 neu bi chan)
#define THU_MUC_GOC  "server_files" // Thu muc goc chua file tren server

// Tai khoan hop le (hardcode don gian)
const string TAI_KHOAN = "admin";
const string MAT_KHAU  = "1234";


// ══════════════════════════════════════════
// BIEN TOAN CUC
// ══════════════════════════════════════════

SOCKET  socketChinh = INVALID_SOCKET;  // Socket chinh dang lang nghe cong 21
bool    dangChay    = false;           // Trang thai server (true = dang chay)


// ══════════════════════════════════════════
// HAM TIEN ICH: GUI / NHAN QUA SOCKET
// ══════════════════════════════════════════

// Gui mot dong phan hoi toi client.
// FTP yeu cau moi phan hoi ket thuc bang \r\n
void guiPhanHoi(SOCKET s, const string& thongBao) {
    string truyen = thongBao + "\r\n";
    send(s, truyen.c_str(), (int)truyen.size(), 0);
}

// Nhan mot dong lenh tu client (doc den khi gap ky tu xuong dong \n)
string nhanLenh(SOCKET s) {
    string ket = "";
    char c;
    while (true) {
        int r = recv(s, &c, 1, 0);
        if (r <= 0) break;       // Ket noi bi dut
        if (c == '\n') break;    // Ket thuc dong (gap \n la xong mot lenh)
        if (c != '\r') ket += c; // Bo \r, chi giu ki tu hop le
    }
    return ket;
}


// ══════════════════════════════════════════
// HAM TAO DATA CONNECTION (Cong ngau nhien)
// ══════════════════════════════════════════

// Tao mot socket tam tren cong ngau nhien de nhan Data Connection tu client.
// Cach lam: bind cong 0 → he dieu hanh tu cap cong trong → lay cong do.
// Tra ve: socket vua tao va so cong (qua bien "cong")
SOCKET taoDataServer(int& cong) {
    SOCKET ds = socket(AF_INET, SOCK_STREAM, 0);  // Tao socket TCP

    sockaddr_in diaChi;
    diaChi.sin_family      = AF_INET;
    diaChi.sin_addr.s_addr = INADDR_ANY;
    diaChi.sin_port        = htons(0);   // Cong 0 = he dieu hanh tu chon cong trong

    bind(ds, (sockaddr*)&diaChi, sizeof(diaChi));

    // Doc so cong vua duoc cap
    sockaddr_in diaChiBound;
    int len = sizeof(diaChiBound);
    getsockname(ds, (sockaddr*)&diaChiBound, &len);
    cong = ntohs(diaChiBound.sin_port);  // Chuyen ve so nguyen de in ra cho client

    listen(ds, 1);  // Cho phep 1 ket noi dang cho
    return ds;
}


// ══════════════════════════════════════════
// HAM LAY DANH SACH FILE TRONG THU MUC
// ══════════════════════════════════════════

// Lay danh sach ten file (chi file, khong lay thu muc con).
// Dung FindFirstFileA / FindNextFileA cua Windows API.
vector<string> layDanhSachFile(const string& tenThuMuc) {
    vector<string> dsFile;

    // Pattern tim kiem: "server_files\docs\*"
    string pattern = string(THU_MUC_GOC) + "\\" + tenThuMuc + "\\*";

    WIN32_FIND_DATAA timKiem;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &timKiem);

    if (hFind == INVALID_HANDLE_VALUE) {
        return dsFile;  // Thu muc khong ton tai → tra ve danh sach rong
    }

    do {
        bool laThuMucCon = (timKiem.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        bool laDauCham   = (string(timKiem.cFileName) == "." || string(timKiem.cFileName) == "..");

        // Chi lay file that su (bo qua "." ".." va thu muc con)
        if (!laThuMucCon && !laDauCham) {
            dsFile.push_back(timKiem.cFileName);
        }
    } while (FindNextFileA(hFind, &timKiem));

    FindClose(hFind);
    return dsFile;
}


// ══════════════════════════════════════════
// XU LY XAC THUC (USER + PASS)
// ══════════════════════════════════════════

// Nhan va kiem tra lenh USER + PASS tu client.
// Tra ve true neu dang nhap dung, false neu sai.
bool xacThuc(SOCKET ctrl) {
    // --- Nhan lenh USER ---
    string lenh = nhanLenh(ctrl);
    cout << "[SERVER] Nhan: " << lenh << "\n";

    if (lenh.size() < 6 || lenh.substr(0, 5) != "USER ") {
        guiPhanHoi(ctrl, "500 Lenh khong hop le");
        return false;
    }
    string tenND = lenh.substr(5);  // Lay phan sau "USER "
    guiPhanHoi(ctrl, "331 Password required");

    // --- Nhan lenh PASS ---
    lenh = nhanLenh(ctrl);
    cout << "[SERVER] Nhan: PASS ****\n";  // An mat khau khi hien thi

    if (lenh.size() < 6 || lenh.substr(0, 5) != "PASS ") {
        guiPhanHoi(ctrl, "500 Lenh khong hop le");
        return false;
    }
    string matKhauNhap = lenh.substr(5);  // Lay phan sau "PASS "

    // --- Kiem tra tai khoan ---
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
// XU LY LENH LIST (Gui danh sach file)
// ══════════════════════════════════════════

void xuLyList(SOCKET ctrl, const string& tenThuMuc) {
    // Kiem tra thu muc co ton tai khong
    string duongDanThuMuc = string(THU_MUC_GOC) + "\\" + tenThuMuc;
    DWORD thuocTinh = GetFileAttributesA(duongDanThuMuc.c_str());
    bool tonTai = (thuocTinh != INVALID_FILE_ATTRIBUTES) &&
                  (thuocTinh & FILE_ATTRIBUTE_DIRECTORY);

    if (!tonTai) {
        guiPhanHoi(ctrl, "550 Directory not found");
        cout << "[SERVER] Thu muc '" << tenThuMuc << "' khong ton tai\n";
        return;
    }

    // Lay danh sach file trong thu muc
    vector<string> dsFile = layDanhSachFile(tenThuMuc);
    cout << "[SERVER] Thu muc '" << tenThuMuc << "' co " << dsFile.size() << " file\n";

    // --- Tao Data Connection ---
    int cong;
    SOCKET ds = taoDataServer(cong);
    // Bao cho client biet cong nao se dung de nhan data
    guiPhanHoi(ctrl, "150 " + to_string(cong) + " Opening data connection");

    // Cho client ket noi vao cong data
    sockaddr_in diaChiClient;
    int lenClient = sizeof(diaChiClient);
    SOCKET dataConn = accept(ds, (sockaddr*)&diaChiClient, &lenClient);

    // --- Gui danh sach file: moi file mot dong, ket thuc bang \r\n ---
    for (const string& tenFile : dsFile) {
        string dong = tenFile + "\r\n";
        send(dataConn, dong.c_str(), (int)dong.size(), 0);
    }

    // Dong Data Connection (bao hieu da gui xong)
    closesocket(dataConn);
    closesocket(ds);

    guiPhanHoi(ctrl, "226 Transfer complete");
    cout << "[SERVER] Da gui xong danh sach file\n";
}


// ══════════════════════════════════════════
// XU LY LENH RETR (Gui noi dung file)
// ══════════════════════════════════════════

void xuLyRetr(SOCKET ctrl, const string& duongDanFile) {
    // Chuan hoa duong dan: thay '/' bang '\' cho Windows
    string duongDanChuan = duongDanFile;
    for (char& c : duongDanChuan) {
        if (c == '/') c = '\\';
    }

    string duongDanDayDu = string(THU_MUC_GOC) + "\\" + duongDanChuan;

    // Mo file de doc (che do nhi phan - de tai duoc ca file anh, zip...)
    ifstream file(duongDanDayDu, ios::binary);
    if (!file.is_open()) {
        guiPhanHoi(ctrl, "550 File not found");
        cout << "[SERVER] File '" << duongDanFile << "' khong ton tai\n";
        return;
    }

    cout << "[SERVER] Dang gui file: " << duongDanFile << "\n";

    // --- Tao Data Connection ---
    int cong;
    SOCKET ds = taoDataServer(cong);
    guiPhanHoi(ctrl, "150 " + to_string(cong) + " Opening data connection");

    // Cho client ket noi vao cong data
    sockaddr_in diaChiClient;
    int lenClient = sizeof(diaChiClient);
    SOCKET dataConn = accept(ds, (sockaddr*)&diaChiClient, &lenClient);

    // --- Doc va gui noi dung file theo tung khoi 4KB ---
    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        int soByte = (int)file.gcount();
        send(dataConn, buffer, soByte, 0);
    }
    file.close();

    // Dong Data Connection (bao hieu da gui xong file)
    closesocket(dataConn);
    closesocket(ds);

    guiPhanHoi(ctrl, "226 Transfer complete");
    cout << "[SERVER] Da gui xong file: " << duongDanFile << "\n";
}


// ══════════════════════════════════════════
// XU LY TUNG CLIENT (chay trong thread rieng)
// ══════════════════════════════════════════

void xuLyClient(SOCKET ctrl) {
    // Gui thong bao chao mung ngay khi client ket noi vao
    guiPhanHoi(ctrl, "220 FTP Server Ready");

    // --- Buoc 1: Xac thuc ---
    if (!xacThuc(ctrl)) {
        closesocket(ctrl);
        return;  // Sai mat khau thi dong ket noi
    }

    // --- Buoc 2: Vong lap nhan lenh ---
    while (true) {
        string lenh = nhanLenh(ctrl);
        if (lenh.empty()) break;  // Ket noi bi dut

        cout << "[SERVER] Nhan lenh: " << lenh << "\n";

        // Kiem tra lenh LIST <ten_thu_muc>
        if (lenh.size() > 5 && lenh.substr(0, 5) == "LIST ") {
            string tenThuMuc = lenh.substr(5);
            xuLyList(ctrl, tenThuMuc);

        // Kiem tra lenh RETR <thu_muc>/<ten_file>
        } else if (lenh.size() > 5 && lenh.substr(0, 5) == "RETR ") {
            string duongDan = lenh.substr(5);
            xuLyRetr(ctrl, duongDan);

        // Kiem tra lenh QUIT
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
// VONG LAP CHAP NHAN KET NOI (chay trong thread)
// ══════════════════════════════════════════

void vongLapAccept() {
    while (dangChay) {
        sockaddr_in diaChiClient;
        int lenClient = sizeof(diaChiClient);

        // Cho client ket noi vao...
        SOCKET clientSocket = accept(socketChinh, (sockaddr*)&diaChiClient, &lenClient);

        if (clientSocket == INVALID_SOCKET) break;  // socketChinh bi dong → thoat

        cout << "[SERVER] Client " << inet_ntoa(diaChiClient.sin_addr) << " da ket noi\n";

        // Moi client duoc xu ly trong mot thread rieng
        thread t(xuLyClient, clientSocket);
        t.detach();  // Cho thread chay doc lap, khong can doi no ket thuc
    }
}


// ══════════════════════════════════════════
// LENH START - Khoi dong server
// ══════════════════════════════════════════

void khoidong() {
    if (dangChay) {
        cout << "[SERVER] Server da dang chay roi!\n";
        return;
    }

    // Tao socket TCP (giong code mau cua thay)
    socketChinh = socket(AF_INET, SOCK_STREAM, 0);
    if (socketChinh == INVALID_SOCKET) {
        cout << "[SERVER] Loi tao socket: " << WSAGetLastError() << "\n";
        return;
    }
    cout << "Socket created.\n";

    // Tranh loi "Address already in use" khi restart nhanh
    int opt = 1;
    setsockopt(socketChinh, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    // Gan dia chi va cong 21 (giong code mau cua thay)
    sockaddr_in serverAddr;
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port        = htons(CONG_FTP);

    if (bind(socketChinh, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "[SERVER] Loi bind cong " << CONG_FTP << ": " << WSAGetLastError() << "\n";
        cout << "         Giai phap: Chay CodeBlocks bang 'Run as administrator'\n";
        cout << "         Hoac doi CONG_FTP thanh 2121 (khong can quyen Admin)\n";
        closesocket(socketChinh);
        socketChinh = INVALID_SOCKET;
        return;
    }
    cout << "Bind done.\n";

    // Lang nghe ket noi den (toi da 5 ket noi cho hang doi)
    listen(socketChinh, 5);

    dangChay = true;
    cout << "[SERVER] Dang lang nghe tren cong " << CONG_FTP << "...\n";

    // Chay vong lap accept trong thread rieng (khong block chuong trinh chinh)
    thread t(vongLapAccept);
    t.detach();
}


// ══════════════════════════════════════════
// LENH STOP - Tat server
// ══════════════════════════════════════════

void tatServer() {
    if (!dangChay) {
        cout << "[SERVER] Server chua chay!\n";
        return;
    }
    dangChay = false;
    closesocket(socketChinh);   // Dong socket → vongLapAccept() se tu thoat
    socketChinh = INVALID_SOCKET;
    cout << "[SERVER] Da tat server.\n";
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

    cout << "========================================\n";
    cout << "      FTP SERVER - DE TAI 13\n";
    cout << "========================================\n";
    cout << "Lenh: start | stop | exit\n";
    cout << "----------------------------------------\n";

    string lenh;
    while (true) {
        cout << "server> ";
        getline(cin, lenh);

        if (lenh == "start") {
            khoidong();

        } else if (lenh == "stop") {
            tatServer();

        } else if (lenh == "exit") {
            if (dangChay) tatServer();
            cout << "Chuong trinh ket thuc.\n";
            break;

        } else if (!lenh.empty()) {
            cout << "Lenh khong duoc ho tro. Go: start | stop | exit\n";
        }
    }

    WSACleanup();  // Don dep Winsock truoc khi thoat (giong code mau thay)
    return 0;
}

