#ifndef SERVER_CONF_H_
#define SERVER_CONF_H_
#include <string>
#include <unordered_map>

using namespace std;

bool readAttribute(const string& strLine, string& name, string& value);


class Account
{
protected:
    string username;
private:
    string password;
/*
    string name;
    time_t start;
    time_t finish;
    bool loggedin;
    unsigned long numIP;
    string ipAddr;
*/
public:
    Account();
    virtual ~Account();
    Account(const string& username);
    const string& getUserName() const
    {
        return this->username;
    }
    void setUserName(const string& username)
    {
        this->username = username;
    }
    void setPassword(const string& pass)
    {
        this->password = pass;
    }
    bool isValidPassword(const string& pass) const
    {
        return this->password==pass;
    }
/*
    void setStartTime(time_t start)
    {
        this->start = start;
    }
    void setFinishTime(time_t finish)
    {
        this->finish = finish;
    }
    time_t getStartTime()
    {
        return start;
    }
    time_t getFinishTime()
    {
        return finish;
    }
    void setLogin(bool login)
    {
        this->loggedin = login;
    }
    bool isLogin()
    {
        return loggedin;
    }
    void setIpNumber(unsigned long ipNum)
    {
        this->numIP = ipNum;
    }
    unsigned long getIpNumber()
    {
        return numIP;
    }
*/
};

class ServerConfig
{
protected:
    int timeout;
    unordered_map<string,Account*> accMap;
public:
    ServerConfig();
    virtual ~ServerConfig();
    void addAccount(Account* acc);
    Account* getAccount(const string& username);
    bool isValidUser(const string& username);
    bool authenticate(const string& username, const string& password);
    //Account* getAccount(int index);
    void removeAccount(const string& username);
    void removeAllAccount();
    void setTimeOut(int t) {timeout = t;}
    int getTimeout() {return timeout;}
    virtual bool loadAccountsFromFile(const string& filename) = 0;
};



#endif // SERVER_CONF_H_
