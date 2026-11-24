#ifndef FTPSERVERCONFIG_H
#define FTPSERVERCONFIG_H

#include "serverconfig.h"
#include <vector>
#include <iostream>

class FTPServerConfig : public ServerConfig
{
private:
    string rootDir;
    unsigned short port;
    string accountFileName;

public:
    FTPServerConfig();
    virtual ~FTPServerConfig();

    bool loadAccountsFromFile(const string& filename) override;
    bool saveAccountsToFile(const string& filename);
    bool addFTPAccount(const string& username, const string& password);
    void printAccounts();
    vector<string> getAccountUsernames();

    const string& getRootDir() const { return rootDir; }
    void setRootDir(const string& dir) { rootDir = dir; }

    unsigned short getPort() const { return port; }
    void setPort(unsigned short p) { port = p; }
};

#endif // FTPSERVERCONFIG_H
