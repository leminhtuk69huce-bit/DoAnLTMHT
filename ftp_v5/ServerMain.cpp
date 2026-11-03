#include "FTPServer.h"
#include <iostream>
#include <string>

using namespace std;

int main() {
    // Khoi tao thu vien Winsock
    if (!khoiTaoWinsock()) {
        return 1;
    }

    cout << "========================================\n";
    cout << "      FTP SERVER - DE TAI 13\n";
    cout << "========================================\n";
    cout << "(Go 'help' de xem danh sach lenh)\n";
    cout << "----------------------------------------\n";

    string lenh;
    while (true) {
        cout << "server> ";
        getline(cin, lenh);

        if (lenh.empty()) continue;

        if (lenh == "start") {
            khoiDongServer();

        } else if (lenh == "stop") {
            tatServer();

        } else if (lenh == "ip") {
            hienThiIPServer();

        } else if (lenh == "help") {
            troGiupServer();

        } else if (lenh == "exit") {
            tatServer();
            cout << "Chuong trinh ket thuc.\n";
            break;

        } else {
            cout << "Lenh khong hop le. Go 'help' de xem huong dan.\n";
        }
    }

    donDepWinsock();
    return 0;
}
