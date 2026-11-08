#include "FTPServer.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

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

    string dong;
    while (true) {
        cout << "server> ";
        getline(cin, dong);

        if (dong.empty()) continue;

        istringstream iss(dong);
        vector<string> phan;
        string tu;
        while (iss >> tu) phan.push_back(tu);

        if (phan.empty()) continue;
        string tenLenh = phan[0];

        if (tenLenh == "start") {
            khoiDongServer();

        } else if (tenLenh == "stop") {
            tatServer();

        } else if (tenLenh == "ip") {
            hienThiIPServer();

        } else if (tenLenh == "adduser") {
            if (phan.size() != 3) {
                cout << "Cach dung: adduser <username> <password>\n";
                cout << "Vi du   : adduser sinhvien 123456\n";
                continue;
            }
            themTaiKhoan(phan[1], phan[2]);
            cout << "[SERVER] Da tao/cap nhat tai khoan '" << phan[1] << "' thanh cong!\n";

        } else if (tenLenh == "users") {
            hienThiDanhSachTaiKhoan();

        } else if (tenLenh == "help") {
            troGiupServer();

        } else if (tenLenh == "exit") {
            tatServer();
            cout << "Chuong trinh ket thuc.\n";
            break;

        } else {
            cout << "Lenh khong hop le: '" << tenLenh << "'. Go 'help' de xem huong dan.\n";
        }
    }

    donDepWinsock();
    return 0;
}

