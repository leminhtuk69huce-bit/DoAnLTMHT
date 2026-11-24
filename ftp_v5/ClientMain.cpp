#include "socket.h"
#include "ftpclientcli.h"
#include <iostream>

// ============================================================================
// CHUONG TRINH CHINH FTP CLIENT (ClientMain.cpp)
// ============================================================================
int main()
{
    try
    {
        // 1. Khoi tao thu vien Winsock theo framework cua Thay
        Socket::startUp();

        // 2. Khoi tao giao dien dong lenh FTP Client CLI
        FTPClientCLI cli;

        // 3. Bat dau vong lap doc va thuc thi lenh cua Client
        cli.run();

        // 4. Don dep thu vien Winsock khi ket thuc chuong trinh
        Socket::cleanUp();
    }
    catch (SocketException& e)
    {
        std::cerr << "[ClientMain] Ngoai le Socket: " << e.what() << std::endl;
        Socket::cleanUp();
        return 1;
    }
    catch (std::exception& e)
    {
        std::cerr << "[ClientMain] Ngoai le he thong: " << e.what() << std::endl;
        Socket::cleanUp();
        return 1;
    }

    return 0;
}
