#include "ftpservercli.h"
#include <iostream>
#include <vector>
#include <winsock2.h>

// Ham tien ich lay danh sach tat ca dia chi IPv4 cua may tinh
static vector<string> getMachineIPList()
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
                string ipStr = inet_ntoa(addr);
                ips.push_back(ipStr);
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

FTPServerCLI::FTPServerCLI() : CmdLineInterface("ftp_server> ")
{
    cout << "==========================================================" << endl;
    cout << "  HE THONG FTP SERVER v5 - BULK DOWNLOAD (DE TAI 13)" << endl;
    cout << "  Xay dung theo Framework huong doi tuong cua Thay Bui Phong" << endl;
    cout << "==========================================================" << endl;

    // Khoi tao cac lenh quan tri CLI
    initCmd();

    // Tao doi tuong FTPServer tren cong 21
    server = new FTPServer(21);
    server->configServer();

    // Tu dong khoi dong Server
    if (server->start())
    {
        cout << "[*] FTP Server da tu dong khoi dong tren cong 21!" << endl;
        
        // Hien thi dia chi IP cua may
        vector<string> ips = getMachineIPList();
        cout << "[*] Dia chi IP cua may: ";
        for (size_t i = 0; i < ips.size(); ++i)
        {
            cout << ips[i];
            if (i + 1 < ips.size()) cout << " | ";
        }
        cout << endl;
        cout << "[*] San sang tiep nhan ket noi tu FTP Client." << endl;
    }
    else
    {
        cout << "[!] Khong the khoi dong Server tu dong. Go 'start' de thu lai." << endl;
    }
    cout << "[*] Go 'help' de xem danh sach lenh quan tri.\n" << endl;
}

FTPServerCLI::~FTPServerCLI()
{
    if (server != NULL)
    {
        delete server;
        server = NULL;
    }
}

void FTPServerCLI::initCmd()
{
    // Dang ky cac ham xu ly lenh voi CLI thong qua CLI_CAST cua Thay
    addCmd("start",   CLI_CAST(&FTPServerCLI::doStart));
    addCmd("stop",    CLI_CAST(&FTPServerCLI::doStop));
    addCmd("status",  CLI_CAST(&FTPServerCLI::doStatus));
    addCmd("ip",      CLI_CAST(&FTPServerCLI::doIP));
    addCmd("adduser", CLI_CAST(&FTPServerCLI::doAddUser));
    addCmd("users",   CLI_CAST(&FTPServerCLI::doUsers));
    addCmd("help",    CLI_CAST(&FTPServerCLI::doHelp));
}

void FTPServerCLI::doStart(string cmd_argv[], int cmd_argc)
{
    if (server->isRunning())
    {
        cout << "[*] Server dang chay tren cong " << server->getFTPConfig()->getPort() << "!" << endl;
    }
    else if (server->start())
    {
        cout << "[+] Khoi dong FTP Server thanh cong tren cong " << server->getFTPConfig()->getPort() << "!" << endl;
    }
    else
    {
        cout << "[-] Khong the khoi dong FTP Server!" << endl;
    }
}

void FTPServerCLI::doStop(string cmd_argv[], int cmd_argc)
{
    if (!server->isRunning())
    {
        cout << "[*] Server hien tai dang dung." << endl;
    }
    else
    {
        server->stop();
        cout << "[-] Da dung FTP Server thanh cong." << endl;
    }
}

void FTPServerCLI::doStatus(string cmd_argv[], int cmd_argc)
{
    vector<string> ips = getMachineIPList();

    cout << "--- TRANG THAI FTP SERVER ---" << endl;
    cout << " Trinh trang:    " << (server->isRunning() ? "DANG HOAT DONG (RUNNING)" : "DANG DUNG (STOPPED)") << endl;
    cout << " Cong lang nghe: " << server->getFTPConfig()->getPort() << endl;
    cout << " Dia chi IP may: ";
    for (size_t i = 0; i < ips.size(); ++i)
    {
        cout << ips[i];
        if (i + 1 < ips.size()) cout << " | ";
    }
    cout << endl;
    cout << " Thu muc goc:    " << server->getFTPConfig()->getRootDir() << endl;
    cout << " So tai khoan:   " << server->getFTPConfig()->getAccountUsernames().size() << endl;
    cout << "-----------------------------" << endl;
}

void FTPServerCLI::doIP(string cmd_argv[], int cmd_argc)
{
    vector<string> ips = getMachineIPList();
    cout << "=== DANH SACH DIA CHI IP CUA MAY SERVER ===" << endl;
    for (size_t i = 0; i < ips.size(); ++i)
    {
        if (ips[i] == "127.0.0.1")
        {
            cout << "  - Loopback (Noi bo may):  " << ips[i] << endl;
        }
        else
        {
            cout << "  - Mang noi bo (LAN):      " << ips[i] << endl;
        }
    }
    cout << "-> Client tren may khac ket noi bang: login " << (ips.size() > 1 ? ips[1] : ips[0]) << " <user> <pass>" << endl;
    cout << "============================================" << endl;
}

void FTPServerCLI::doAddUser(string cmd_argv[], int cmd_argc)
{
    if (cmd_argc < 3)
    {
        cout << "Cu phap: adduser <username> <password>" << endl;
        cout << "Vi du:  adduser sinhvien 123456" << endl;
        return;
    }

    string user = cmd_argv[1];
    string pass = cmd_argv[2];

    if (server->getFTPConfig()->addFTPAccount(user, pass))
    {
        cout << "[+] Them/cap nhat tai khoan '" << user << "' thanh cong!" << endl;
    }
    else
    {
        cout << "[-] Them tai khoan that bai!" << endl;
    }
}

void FTPServerCLI::doUsers(string cmd_argv[], int cmd_argc)
{
    server->getFTPConfig()->printAccounts();
}

void FTPServerCLI::doHelp(string cmd_argv[], int cmd_argc)
{
    cout << "=== HUONG DAN LENH QUAN TRI FTP SERVER ===" << endl;
    cout << "  start                 : Khoi dong FTP Server lang nghe ket noi" << endl;
    cout << "  stop                  : Dung FTP Server" << endl;
    cout << "  status                : Xem trang thai Server (hien thi ca IP va Cong)" << endl;
    cout << "  ip                    : Xem danh sach tat ca dia chi IP cua may Server" << endl;
    cout << "  adduser <user> <pass> : Them tai khoan nguoi dung moi" << endl;
    cout << "  users                 : Xem danh sach cac tai khoan da dang ky" << endl;
    cout << "  help                  : Hien thi huong dan nay" << endl;
    cout << "  quit                  : Thoat chuong trinh Server" << endl;
    cout << "==========================================" << endl;
}
