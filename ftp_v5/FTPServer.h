#ifndef FTPSERVER_H
#define FTPSERVER_H

#include "tcpserver.h"
#include "ftpsession.h"
#include "ftpserverconfig.h"

// ============================================================================
// LOP FTPServer: Ke thua tu TCPServer (Framework cua Thay Bui Phong)
// Dai dien cho Server ung dung FTP, quan ly cong ket noi va cac phien (Session)
// ============================================================================
class FTPServer : public TCPServer
{
public:
    // Khoi tao Server voi cong mac dinh la 21 (Cong FTP tieu chuan)
    FTPServer(unsigned short port = 21);
    virtual ~FTPServer();

    // Nap cau hinh Server va danh sach tai khoan
    bool configServer() override;

    // Lay con tro den cau hinh Server
    FTPServerConfig* getFTPConfig() { return static_cast<FTPServerConfig*>(conf); }

protected:
    // Dang ky bang cac lenh FTP ma Server ho tro (USER, PASS, PASV, LIST, RETR, QUIT)
    void initCmd() override;

    // Khoi tao va dieu phoi phien lam viec moi cho tung Client ket noi den
    void startNewSession(TcpSocket slave) override;

private:
    // Doc dong lenh tu Client qua kenh dieu khien (slave socket)
    int readCmd(TcpSocket& slave, string& cmdLine);

    // Phan tich dong lenh thanh ten lenh va cac tham so (cmd_argv, cmd_argc)
    unsigned short parseCmd(const string& cmdLine, string cmd_argv[], int& cmd_argc);
};

#endif // FTPSERVER_H
