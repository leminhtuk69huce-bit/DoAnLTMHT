#include "ftpserverconfig.h"
#include <fstream>
#include <sstream>

FTPServerConfig::FTPServerConfig()
{
    this->rootDir = "server_files";
    this->port = 21;
    this->timeout = 0;
    this->accountFileName = "accounts.txt";

    // Nap tai khoan mac dinh ban dau
    addFTPAccount("admin", "admin123");
    addFTPAccount("student", "huce123");
    addFTPAccount("guest", "guest");

    // Thu nap them tu file neu co
    loadAccountsFromFile(accountFileName);
}

FTPServerConfig::~FTPServerConfig()
{
}

bool FTPServerConfig::loadAccountsFromFile(const string& filename)
{
    ifstream f(filename.c_str());
    if (!f.is_open())
    {
        return false;
    }

    string line;
    while (getline(f, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        istringstream iss(line);
        string user, pass;
        if (iss >> user >> pass)
        {
            addFTPAccount(user, pass);
        }
    }
    f.close();
    return true;
}

bool FTPServerConfig::saveAccountsToFile(const string& filename)
{
    ofstream f(filename.c_str());
    if (!f.is_open())
        return false;

    for (auto const& pair : accMap)
    {
        // Account has username
        f << pair.first << "\n";
    }
    f.close();
    return true;
}

bool FTPServerConfig::addFTPAccount(const string& username, const string& password)
{
    if (username.empty() || password.empty())
        return false;

    Account* existing = getAccount(username);
    if (existing != NULL)
    {
        existing->setPassword(password);
        return true;
    }

    Account* acc = new Account(username);
    acc->setPassword(password);
    addAccount(acc);
    return true;
}

void FTPServerConfig::printAccounts()
{
    cout << "=== DANH SACH TAI KHOAN FTP DA DANG KY ===" << endl;
    int index = 1;
    for (auto const& pair : accMap)
    {
        cout << "  " << index++ << ". " << pair.first << endl;
    }
    cout << "==========================================" << endl;
}

vector<string> FTPServerConfig::getAccountUsernames()
{
    vector<string> list;
    for (auto const& pair : accMap)
    {
        list.push_back(pair.first);
    }
    return list;
}
