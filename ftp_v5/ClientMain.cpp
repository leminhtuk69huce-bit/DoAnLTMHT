#include "FTPClient.h"
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
    cout << "      FTP CLIENT - DE TAI 13\n";
    cout << "========================================\n";
    cout << "(Go 'help' de xem danh sach lenh)\n";
    cout << "----------------------------------------\n";

    string dong;
    while (true) {
        cout << "client> ";
        getline(cin, dong);

        if (dong.empty()) continue;

        istringstream iss(dong);
        vector<string> phan;
        string tu;
        while (iss >> tu) phan.push_back(tu);

        if (phan.empty()) continue;
        string tenLenh = phan[0];

        if (tenLenh == "login") {
            if (phan.size() != 4) {
                cout << "Cach dung: login <IP> <username> <password>\n";
                cout << "Vi du   : login 127.0.0.1 admin 1234\n";
                continue;
            }
            dangNhap(phan[1], phan[2], phan[3]);

        } else if (tenLenh == "mget") {
            if (phan.size() != 2) {
                cout << "Cach dung: mget <ten_thu_muc>\n";
                cout << "Vi du   : mget docs\n";
                continue;
            }
            taiToanBoThuMuc(phan[1]);

        } else if (tenLenh == "help") {
            troGiupClient();

        } else if (tenLenh == "quit") {
            ketThucPhien();
            break;

        } else {
            cout << "Lenh khong hop le: '" << tenLenh << "'. Go 'help' de xem huong dan.\n";
        }
    }

    donDepWinsock();
    return 0;
}
