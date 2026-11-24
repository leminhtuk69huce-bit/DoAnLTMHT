#include "socket.h"
#include "ftpservercli.h"
#include <iostream>

// ============================================================================
// CHUONG TRINH CHINH FTP SERVER (ServerMain.cpp)
// ============================================================================
int main()
{
    try
    {
        // 1. Khoi tao thu vien Winsock (goi ham tinh cua lop Socket theo chuan cua Thay)
        Socket::startUp();

        // 2. Khoi tao giao dien dong lenh quan tri FTP Server CLI
        FTPServerCLI cli;

        // 3. Bat dau vong lap doc va thuc thi lenh quan tri
        cli.run();

        // 4. Don dep thu vien Winsock khi ket thuc chuong trinh
        Socket::cleanUp();
    }
    catch (SocketException& e)
    {
        std::cerr << "[ServerMain] Ngoai le Socket: " << e.what() << std::endl;
        Socket::cleanUp();
        return 1;
    }
    catch (std::exception& e)
    {
        std::cerr << "[ServerMain] Ngoai le he thong: " << e.what() << std::endl;
        Socket::cleanUp();
        return 1;
    }

    return 0;
}
