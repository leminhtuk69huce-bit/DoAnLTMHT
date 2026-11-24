#ifndef FTPSERVERCLI_H
#define FTPSERVERCLI_H

#include "cli.h"
#include "ftpserver.h"

// ============================================================================
// LOP FTPServerCLI: Ke thua tu CmdLineInterface (CLI cua Thay)
// Giao dien dong lenh tuong tac giup Quan tri vien quan ly Server
// ============================================================================
class FTPServerCLI : public CmdLineInterface
{
private:
    FTPServer* server; // Con tro quan ly Server FTP

public:
    FTPServerCLI();
    virtual ~FTPServerCLI();

protected:
    // Dang ky cac lenh quan tri cua Server (start, stop, adduser, users, status, help)
    void initCmd() override;

private:
    // Cac ham thuc thi lenh quan tri
    void doStart(string cmd_argv[], int cmd_argc);
    void doStop(string cmd_argv[], int cmd_argc);
    void doStatus(string cmd_argv[], int cmd_argc);
    void doIP(string cmd_argv[], int cmd_argc);
    void doAddUser(string cmd_argv[], int cmd_argc);
    void doUsers(string cmd_argv[], int cmd_argc);
    void doHelp(string cmd_argv[], int cmd_argc);
};

#endif // FTPSERVERCLI_H
