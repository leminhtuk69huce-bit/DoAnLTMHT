#ifndef FTPSESSION_H
#define FTPSESSION_H

#include "session.h"
#include "tcpserversocket.h"
#include "ftpserverconfig.h"
#include <string>
#include <vector>

using namespace std;

class FTPSession : public Session
{
private:
    bool isLoggedIn;
    string currentUser;
    string pendingUser;
    TcpServerSocket* dataServer;
    unsigned short dataPort;

public:
    FTPSession(const TcpSocket& slave, ServerConfig* conf);
    virtual ~FTPSession();

    void doUser(string cmd_argv[], int cmd_argc);
    void doPass(string cmd_argv[], int cmd_argc);
    void doPasv(string cmd_argv[], int cmd_argc);
    void doList(string cmd_argv[], int cmd_argc);
    void doRetr(string cmd_argv[], int cmd_argc);
    void doQuit(string cmd_argv[], int cmd_argc);
    void doNoop(string cmd_argv[], int cmd_argc);
    void doUnknown(string cmd_argv[], int cmd_argc) override;

private:
    FTPServerConfig* getFTPConfig();
    void sendResp(const string& msg);
    vector<string> scanDirectory(const string& folder);
    void closeDataServer();
};

#endif // FTPSESSION_H
