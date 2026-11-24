#include "ftpclientcli.h"
#include <iostream>
#include <vector>
#include <winsock2.h>

// Ham tien ich lay danh sach dia chi IP cua may tinh
static vector<string> getClientIPList()
{
    vector<string> ips;
    try
    {
        string host = Socket::getHostname();
        hostent* pHost = gethostbyname(host.c_str());
        if (pHost != NULL)
        {
            for (int i = 0; pHost->h_addr_list[i] != NULL; ++i)
            {
                in_addr addr;
                addr.s_addr = *((unsigned long*)pHost->h_addr_list[i]);
                ips.push_back(inet_ntoa(addr));
            }
        }
    }
    catch (...) {}

    if (ips.empty())
    {
        ips.push_back("127.0.0.1");
    }
    return ips;
}

FTPClientCLI::FTPClientCLI() : CmdLineInterface("ftp_client> ")
{
    cout << "==========================================================" << endl;
    cout << "  CHUONG TRINH FTP CLIENT v5 - BULK DOWNLOAD (DE TAI 13)" << endl;
    cout << "  Xay dung theo Framework huong doi tuong cua Thay Bui Phong" << endl;
    cout << "==========================================================" << endl;
    cout << "[*] Go 'help' de xem danh sach cac lenh ho tro." << endl;
    cout << "[*] Mau dang nhap: login 127.0.0.1 admin admin123" << endl;
    cout << "==========================================================\n" << endl;

    initCmd();
}

FTPClientCLI::~FTPClientCLI()
{
}

void FTPClientCLI::initCmd()
{
    // Dang ky cac ham xu ly lenh voi CLI bang cach ep kieu CLI_CAST cua Thay
    addCmd("login", CLI_CAST(&FTPClientCLI::doLogin));
    addCmd("list",  CLI_CAST(&FTPClientCLI::doList));
    addCmd("get",   CLI_CAST(&FTPClientCLI::doGet));
    addCmd("mget",  CLI_CAST(&FTPClientCLI::doMGet));
    addCmd("ip",    CLI_CAST(&FTPClientCLI::doIP));
    addCmd("help",  CLI_CAST(&FTPClientCLI::doHelp));
}

void FTPClientCLI::doLogin(string cmd_argv[], int cmd_argc)
{
    // Cu phap: login <ip> <user> <pass> [port]
    if (cmd_argc < 4)
    {
        cout << "Cu phap: login <ip> <user> <pass> [port]" << endl;
        cout << "Vi du:  login 127.0.0.1 admin admin123" << endl;
        cout << "        login 127.0.0.1 student huce123 21" << endl;
        return;
    }

    string host = cmd_argv[1];
    string user = cmd_argv[2];
    string pass = cmd_argv[3];
    unsigned short port = 21;

    if (cmd_argc >= 5)
    {
        port = (unsigned short)atoi(cmd_argv[4].c_str());
    }

    client.login(host, port, user, pass);
}

void FTPClientCLI::doList(string cmd_argv[], int cmd_argc)
{
    string folder = "";
    if (cmd_argc >= 2)
    {
        folder = cmd_argv[1];
    }

    vector<string> items = client.listFiles(folder);
    if (!items.empty())
    {
        cout << "\n--- DANH SACH TEP TIN TREN SERVER ---" << endl;
        for (size_t i = 0; i < items.size(); ++i)
        {
            cout << "  " << (i + 1) << ". " << items[i] << endl;
        }
        cout << "-------------------------------------\n" << endl;
    }
}

void FTPClientCLI::doGet(string cmd_argv[], int cmd_argc)
{
    if (cmd_argc < 2)
    {
        cout << "Cu phap: get <ten_file_hoac_duong_dan>" << endl;
        cout << "Vi du:  get tailieu.txt" << endl;
        cout << "        get docs/baocao.docx" << endl;
        return;
    }

    string remotePath = cmd_argv[1];
    client.downloadFile(remotePath);
}

void FTPClientCLI::doMGet(string cmd_argv[], int cmd_argc)
{
    string folder = "";
    if (cmd_argc >= 2)
    {
        folder = cmd_argv[1];
    }

    // Goi tinh nang tai toan bo thu muc cua FTPClient
    client.bulkDownload(folder);
}

void FTPClientCLI::doIP(string cmd_argv[], int cmd_argc)
{
    vector<string> ips = getClientIPList();
    cout << "=== DIA CHI IP CUA MAY CLIENT ===" << endl;
    for (size_t i = 0; i < ips.size(); ++i)
    {
        cout << "  - " << ips[i] << endl;
    }
    cout << "=================================" << endl;
}

void FTPClientCLI::doHelp(string cmd_argv[], int cmd_argc)
{
    cout << "=== HUONG DAN SU DUNG FTP CLIENT v5 ===" << endl;
    cout << "  login <ip> <user> <pass> [port] : Ket noi va dang nhap vao Server (Port mac dinh: 21)" << endl;
    cout << "  list [folder]                   : Xem danh sach tep tin/thu muc tren Server" << endl;
    cout << "  get <file_path>                 : Tai 1 tep tin don le tu Server ve may" << endl;
    cout << "  mget [folder]                   : [DE TAI 13] Tai toan bo mot thu muc (Bulk Download)" << endl;
    cout << "  ip                              : Xem dia chi IP cua may nay" << endl;
    cout << "  help                            : Hien thi huong dan nay" << endl;
    cout << "  quit                            : Thoat chuong trinh Client" << endl;
    cout << "========================================" << endl;
}
