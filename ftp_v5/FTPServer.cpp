#include "FTPServer.h"
#include <fstream>
#include <thread>
#include <map>

using namespace std;
#pragma comment(lib, "ws2_32.lib")

map<string, string> dsTaiKhoan = {
    {"admin", "1234"},
    {"user1", "1234"}
};

SOCKET socketChinh = INVALID_SOCKET;
bool dangChay = false;

bool khoiTaoWinsock() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        return false;
    }
    CreateDirectoryA(THU_MUC_GOC, NULL);
    return true;
}

void donDepWinsock() {
    WSACleanup();
}

void guiPhanHoi(SOCKET s, const string& thongBao) {
    string truyen = thongBao + "\r\n";
    send(s, truyen.c_str(), (int)truyen.size(), 0);
}

string nhanLenh(SOCKET s) {
    string ketQua = "";
    char c;
    while (recv(s, &c, 1, 0) > 0) {
        if (c == '\r') continue;
        if (c == '\n') break;
        ketQua += c;
    }
    return ketQua;
}

void troGiupServer() {
    cout << "\nDanh sach lenh Server:\n";
    cout << "  start : Khoi dong FTP Server tren cong 21\n";
    cout << "  stop  : Tat FTP Server\n";
    cout << "  help  : Xem huong dan\n";
    cout << "  exit  : Thoat chuong trinh\n\n";
}

void khoiDongServer() {
    if (dangChay) {
        cout << "[SERVER] Server da dang chay roi!\n";
        return;
    }

    socketChinh = socket(AF_INET, SOCK_STREAM, 0);
    if (socketChinh == INVALID_SOCKET) {
        cout << "[SERVER] Loi: Khong the tao socket\n";
        return;
    }

    sockaddr_in diaChiServer;
    diaChiServer.sin_family = AF_INET;
    diaChiServer.sin_addr.s_addr = INADDR_ANY;
    diaChiServer.sin_port = htons(CONG_FTP);

    if (bind(socketChinh, (sockaddr*)&diaChiServer, sizeof(diaChiServer)) == SOCKET_ERROR) {
        cout << "[SERVER] Loi: Khong the bind cong 21 (vui long chay bang quyen Admin)\n";
        closesocket(socketChinh);
        return;
    }

    if (listen(socketChinh, SOMAXCONN) == SOCKET_ERROR) {
        cout << "[SERVER] Loi: Khong the listen\n";
        closesocket(socketChinh);
        return;
    }

    dangChay = true;
    cout << "[SERVER] Server dang hoat dong tren cong 21\n";

    thread(vongLapAccept).detach();
}

void tatServer() {
    if (!dangChay) {
        cout << "[SERVER] Server hien chua khoi dong.\n";
        return;
    }

    dangChay = false;
    if (socketChinh != INVALID_SOCKET) {
        closesocket(socketChinh);
        socketChinh = INVALID_SOCKET;
    }
    cout << "[SERVER] Da tat Server.\n";
}

SOCKET taoDataServer(int& cong) {
    SOCKET ds = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0;

    bind(ds, (sockaddr*)&addr, sizeof(addr));

    int len = sizeof(addr);
    getsockname(ds, (sockaddr*)&addr, &len);
    cong = ntohs(addr.sin_port);

    listen(ds, 1);
    return ds;
}

bool xacThuc(SOCKET ctrl) {
    string lenh = nhanLenh(ctrl);
    if (lenh.size() < 6 || lenh.substr(0, 5) != "USER ") {
        guiPhanHoi(ctrl, "500 Lenh khong hop le");
        return false;
    }
    string tenND = lenh.substr(5);
    guiPhanHoi(ctrl, "331 Password required");

    lenh = nhanLenh(ctrl);
    if (lenh.size() < 6 || lenh.substr(0, 5) != "PASS ") {
        guiPhanHoi(ctrl, "500 Lenh khong hop le");
        return false;
    }
    string matKhauNhap = lenh.substr(5);

    auto it = dsTaiKhoan.find(tenND);
    if (it != dsTaiKhoan.end() && it->second == matKhauNhap) {
        guiPhanHoi(ctrl, "230 Login successful");
        cout << "[SERVER] Xac thuc thanh cong user: " << tenND << "\n";
        return true;
    } else {
        guiPhanHoi(ctrl, "530 Login incorrect");
        cout << "[SERVER] Xac thuc that bai user: " << tenND << "\n";
        return false;
    }
}

void xuLyRetr(SOCKET ctrl, const string& tenFile) {
    string duongDanChuan = tenFile;
    for (char& c : duongDanChuan) {
        if (c == '/') c = '\\';
    }
    if (duongDanChuan.rfind("server_files\\", 0) == 0) {
        duongDanChuan = duongDanChuan.substr(13);
    }

    string duongDanDayDu = string(THU_MUC_GOC) + "\\" + duongDanChuan;

    ifstream file(duongDanDayDu, ios::binary);
    if (!file.is_open()) {
        guiPhanHoi(ctrl, "550 File not found");
        cout << "[SERVER] File '" << tenFile << "' khong ton tai\n";
        return;
    }

    cout << "[SERVER] Dang truyen file: " << tenFile << "\n";

    int cong;
    SOCKET ds = taoDataServer(cong);
    guiPhanHoi(ctrl, "150 " + to_string(cong) + " Opening data connection");

    sockaddr_in diaChiClient;
    int lenClient = sizeof(diaChiClient);
    SOCKET dataConn = accept(ds, (sockaddr*)&diaChiClient, &lenClient);

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        int soByte = (int)file.gcount();
        if (soByte <= 0) break;
        send(dataConn, buffer, soByte, 0);
    }
    file.close();

    closesocket(dataConn);
    closesocket(ds);

    guiPhanHoi(ctrl, "226 Transfer complete");
    cout << "[SERVER] Da truyen xong file: " << tenFile << "\n";
}

void xuLyClient(SOCKET ctrl) {
    guiPhanHoi(ctrl, "220 FTP Server Ready");

    if (!xacThuc(ctrl)) {
        closesocket(ctrl);
        return;
    }

    while (dangChay) {
        string lenh = nhanLenh(ctrl);
        if (lenh.empty()) break;

        cout << "[SERVER] Lenh tu client: " << lenh << "\n";

        if (lenh.size() >= 5 && lenh.substr(0, 5) == "RETR ") {
            string tenFile = lenh.substr(5);
            xuLyRetr(ctrl, tenFile);
        } else if (lenh == "QUIT") {
            guiPhanHoi(ctrl, "221 Goodbye");
            break;
        } else {
            guiPhanHoi(ctrl, "502 Command not implemented");
        }
    }

    closesocket(ctrl);
    cout << "[SERVER] Client da ngat ket noi\n";
}

void vongLapAccept() {
    while (dangChay) {
        sockaddr_in diaChiClient;
        int len = sizeof(diaChiClient);
        SOCKET ctrl = accept(socketChinh, (sockaddr*)&diaChiClient, &len);

        if (ctrl == INVALID_SOCKET) {
            if (!dangChay) break;
            continue;
        }

        cout << "[SERVER] Client moi ket noi\n";
        thread(xuLyClient, ctrl).detach();
    }
}
