#ifndef FTPCLIENTCLI_H
#define FTPCLIENTCLI_H

#include "cli.h"
#include "ftpclient.h"

// ============================================================================
// LOP FTPClientCLI: Ke thua tu CmdLineInterface (CLI cua Thay)
// Giao dien dong lenh tuong tac giup Nguoi dung thao tac voi FTP Client
// ============================================================================
class FTPClientCLI : public CmdLineInterface
{
private:
    FTPClient client; // Doi tuong FTPClient thuc thi cac chuc nang mang

public:
    FTPClientCLI();
    virtual ~FTPClientCLI();

protected:
    // Dang ky cac lenh nguoi dung: login, list, get, mget, help, quit
    void initCmd() override;

private:
    // Cac ham thuc thi lenh
    void doLogin(string cmd_argv[], int cmd_argc);
    void doList(string cmd_argv[], int cmd_argc);
    void doGet(string cmd_argv[], int cmd_argc);
    void doMGet(string cmd_argv[], int cmd_argc);
    void doIP(string cmd_argv[], int cmd_argc);
    void doHelp(string cmd_argv[], int cmd_argc);
};

#endif // FTPCLIENTCLI_H
