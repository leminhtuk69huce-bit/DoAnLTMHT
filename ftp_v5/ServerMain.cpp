#include "FTPServer.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

int main() {
    if (!khoiTaoWinsock()) {
        return 1;
    }

    cout << "========================================\n";
    cout << "      FTP SERVER - SIMPLE VERSION\n";
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
        } else if (tenLenh == "help") {
            troGiupServer();
        } else if (tenLenh == "exit") {
            tatServer();
            break;
        } else {
            cout << "Lenh khong hop le: '" << tenLenh << "'. Go 'help' de xem huong dan.\n";
        }
    }

    donDepWinsock();
    return 0;
}
