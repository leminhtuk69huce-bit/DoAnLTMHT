#include "ftpsession.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>

FTPSession::FTPSession(const TcpSocket& slave, ServerConfig* conf)
    : Session(slave, conf)
{
    this->isLoggedIn = false;
    this->currentUser = "";
    this->pendingUser = "";
    this->dataServer = NULL;
    this->dataPort = 0;
}

FTPSession::~FTPSession()
{
    closeDataServer();
}

FTPServerConfig* FTPSession::getFTPConfig()
{
    return static_cast<FTPServerConfig*>(this->conf);
}

void FTPSession::sendResp(const string& msg)
{
    try
    {
        this->slave.send(msg);
    }
    catch (SocketException& e)
    {
        cerr << "[Session] Loi send: " << e.what() << endl;
    }
}

void FTPSession::closeDataServer()
{
    if (dataServer != NULL)
    {
        try
        {
            dataServer->close();
        }
        catch (...) {}
        delete dataServer;
        dataServer = NULL;
        dataPort = 0;
    }
}

void FTPSession::doUser(string cmd_argv[], int cmd_argc)
{
    if (cmd_argc < 2)
    {
        sendResp("501 Syntax error in parameters or arguments.\r\n");
        return;
    }
    pendingUser = cmd_argv[1];
    sendResp("331 User name okay, need password.\r\n");
}

void FTPSession::doPass(string cmd_argv[], int cmd_argc)
{
    if (pendingUser.empty())
    {
        sendResp("503 Bad sequence of commands (send USER first).\r\n");
        return;
    }
    string pass = "";
    if (cmd_argc >= 2)
    {
        pass = cmd_argv[1];
    }

    if (getFTPConfig()->authenticate(pendingUser, pass))
    {
        isLoggedIn = true;
        currentUser = pendingUser;
        pendingUser = "";
        sendResp("230 User logged in, proceed.\r\n");
        cout << "[FTP Server] Nguoi dung '" << currentUser << "' dang nhap thanh cong." << endl;
    }
    else
    {
        pendingUser = "";
        sendResp("530 Not logged in, invalid credentials.\r\n");
        cout << "[FTP Server] Dang nhap that bai voi tai khoan '" << pendingUser << "'." << endl;
    }
}

void FTPSession::doPasv(string cmd_argv[], int cmd_argc)
{
    if (!isLoggedIn)
    {
        sendResp("530 Please login with USER and PASS first.\r\n");
        return;
    }

    closeDataServer();

    // Thu mo mot cong nghe thu dong trong dai 20000 - 25000
    static unsigned short nextPort = 20000;
    bool bound = false;
    for (int i = 0; i < 50; i++)
    {
        unsigned short p = nextPort++;
        if (nextPort > 25000) nextPort = 20000;
        try
        {
            dataServer = new TcpServerSocket();
            dataServer->setListen(p, 5);
            dataPort = p;
            bound = true;
            break;
        }
        catch (...)
        {
            delete dataServer;
            dataServer = NULL;
        }
    }

    if (!bound || dataServer == NULL)
    {
        sendResp("425 Can't open passive connection.\r\n");
        return;
    }

    unsigned char p1 = (dataPort >> 8) & 0xFF;
    unsigned char p2 = dataPort & 0xFF;

    ostringstream oss;
    oss << "227 Entering Passive Mode (127,0,0,1," << (int)p1 << "," << (int)p2 << ").\r\n";
    sendResp(oss.str());
}

vector<string> FTPSession::scanDirectory(const string& relFolder)
{
    vector<string> result;
    string basePath = getFTPConfig()->getRootDir();

    string targetDir = basePath;
    if (!relFolder.empty() && relFolder != ".")
    {
        targetDir += "/" + relFolder;
    }

    string searchPattern = targetDir + "/*";

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPattern.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            string name = findData.cFileName;
            if (name == "." || name == "..")
                continue;

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // Thư mục
                result.push_back(name + "/");
            }
            else
            {
                // File thường
                result.push_back(name);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
    return result;
}

void FTPSession::doList(string cmd_argv[], int cmd_argc)
{
    if (!isLoggedIn)
    {
        sendResp("530 Please login with USER and PASS first.\r\n");
        return;
    }

    if (dataServer == NULL)
    {
        sendResp("425 Use PASV first.\r\n");
        return;
    }

    sendResp("150 Here comes the directory listing.\r\n");

    try
    {
        TcpSocket dataSocket = dataServer->accept(10000);
        string folder = "";
        if (cmd_argc >= 2)
        {
            folder = cmd_argv[1];
        }

        vector<string> entries = scanDirectory(folder);
        string listing;
        for (size_t i = 0; i < entries.size(); ++i)
        {
            listing += entries[i] + "\r\n";
        }

        if (!listing.empty())
        {
            dataSocket.send(listing.c_str(), (int)listing.size());
        }

        dataSocket.close();
        closeDataServer();
        sendResp("226 Directory send OK.\r\n");
    }
    catch (SocketException& e)
    {
        closeDataServer();
        sendResp("426 Connection closed; transfer aborted.\r\n");
        cerr << "[FTP Server] Loi du lieu LIST: " << e.what() << endl;
    }
}

void FTPSession::doRetr(string cmd_argv[], int cmd_argc)
{
    if (!isLoggedIn)
    {
        sendResp("530 Please login with USER and PASS first.\r\n");
        return;
    }

    if (cmd_argc < 2)
    {
        sendResp("501 Syntax error in parameters or arguments.\r\n");
        return;
    }

    if (dataServer == NULL)
    {
        sendResp("425 Use PASV first.\r\n");
        return;
    }

    string relPath = cmd_argv[1];
    // Chuyen \ thanh /
    for (size_t i = 0; i < relPath.size(); i++)
    {
        if (relPath[i] == '\\') relPath[i] = '/';
    }

    string fullPath = getFTPConfig()->getRootDir() + "/" + relPath;

    ifstream file(fullPath.c_str(), ios::binary | ios::ate);
    if (!file.is_open())
    {
        closeDataServer();
        sendResp("550 File not found or access denied.\r\n");
        return;
    }

    streamsize fileSize = file.tellg();
    file.seekg(0, ios::beg);

    ostringstream oss;
    oss << "150 Opening BINARY mode data connection for " << relPath << " (" << fileSize << " bytes).\r\n";
    sendResp(oss.str());

    try
    {
        TcpSocket dataSocket = dataServer->accept(10000);

        char buffer[4096];
        while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0)
        {
            int bytesToSend = (int)file.gcount();
            int sent = dataSocket.send(buffer, bytesToSend);
            if (sent <= 0) break;
        }
        file.close();

        dataSocket.close();
        closeDataServer();
        sendResp("226 Transfer complete.\r\n");
        cout << "[FTP Server] Da truyen thanh cong file: " << relPath << " (" << fileSize << " bytes) cho " << currentUser << endl;
    }
    catch (SocketException& e)
    {
        file.close();
        closeDataServer();
        sendResp("426 Connection closed; transfer aborted.\r\n");
        cerr << "[FTP Server] Loi truyen file RETR: " << e.what() << endl;
    }
}

void FTPSession::doQuit(string cmd_argv[], int cmd_argc)
{
    sendResp("221 Goodbye.\r\n");
    quitSession = true;
    cout << "[FTP Server] Nguoi dung '" << (currentUser.empty() ? "Client" : currentUser) << "' da ngat ket noi." << endl;
}

void FTPSession::doNoop(string cmd_argv[], int cmd_argc)
{
    sendResp("200 OK\r\n");
}

void FTPSession::doUnknown(string cmd_argv[], int cmd_argc)
{
    sendResp("500 Syntax error, command unrecognized.\r\n");
}
