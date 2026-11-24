#include <cstring>
#include <iostream>
#include "serverconfig.h"


bool readAttribute(const string& strLine, string& name, string& value)
{
    size_t p = strLine.find("=");
    if(p!=string::npos)
    {
        name = strLine.substr(0,p);
        value = strLine.substr(p+1);
        return true;
    }
    else
        return false;
}

Account::Account()
{
}

Account::~Account()
{

}

Account::Account(const string& user)
{
    this->username = user;
}

void ServerConfig::addAccount(Account* acc)
{
    string username = acc->getUserName();
    if(!username.empty())
    {
        try
        {
            accMap.insert({username,acc});
        }
        catch (exception& e)
        {
            cerr << e.what() << endl;
        }
    }
}


ServerConfig::ServerConfig()
{
}

ServerConfig::~ServerConfig()
{
    // remove all account from account Map and delete account object
    removeAllAccount();
}

Account* ServerConfig::getAccount(const string& username)
{
    try
    {
        unordered_map<string,Account*>::const_iterator got = accMap.find(username);
        if (got == accMap.end())
            return NULL;
        else
            return got->second;
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
        return NULL;
    }
}

bool ServerConfig::isValidUser(const string& username)
{
    return !(accMap.find(username)==accMap.end());
}

bool ServerConfig::authenticate(const string& username, const string& pass)
{
    try
    {
        unordered_map<string,Account*>::const_iterator it = accMap.find(username);
        if(it==accMap.end())
        {
            return false;
        } else
            return it->second->isValidPassword(pass);
    }
    catch(exception& e)
    {
        cerr << e.what() << endl;
        return false;
    }
}

void ServerConfig::removeAccount(const string& username)
{
    try
    {
        accMap.erase(username);
    }
    catch(exception& e)
    {
        cerr << e.what() << endl;
    }
}

void ServerConfig::removeAllAccount()
{
    Account* acc;
    for (auto& x: accMap)
    {
        acc = x.second;
        delete acc;
    }
    accMap.clear();
}


