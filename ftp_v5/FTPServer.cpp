#include "ftpserver.h"
#include <iostream>
#include <sstream>

// ============================================================================
// Trien khai lop FTPServer ke thua tu TCPServer
// ============================================================================

FTPServer::FTPServer(unsigned short port) : TCPServer(port)
{
    // 1. Khoi tao bang lenh ma Server ho tro
    initCmd();

    // 2. Khoi tao doi tuong quan ly cau hinh va tai khoan FTP
    conf = new FTPServerConfig();
}

FTPServer::~FTPServer()
{
    if (isRunning())
    {
        stop();
    }
}

bool FTPServer::configServer()
{
    // Nap tai khoan nguoi dung tu tap tin cau hinh
    cout << "[FTP Server] Nap cau hinh thanh cong. Thu muc goc: " 
         << getFTPConfig()->getRootDir() << endl;
    return true;
}

void FTPServer::initCmd()
{
    // Dang ky cac lenh FTP chuan theo kien truc cua Thay
    // Su dung FUNC_CAST de ep kieu con tro ham thanh vien ve CMD_FUNC
    addCmd("USER", FUNC_CAST(&FTPSession::doUser));
    addCmd("PASS", FUNC_CAST(&FTPSession::doPass));
    addCmd("PASV", FUNC_CAST(&FTPSession::doPasv));
    addCmd("LIST", FUNC_CAST(&FTPSession::doList));
    addCmd("RETR", FUNC_CAST(&FTPSession::doRetr));
    addCmd("QUIT", FUNC_CAST(&FTPSession::doQuit));
    addCmd("NOOP", FUNC_CAST(&FTPSession::doNoop));
}

int FTPServer::readCmd(TcpSocket& slave, string& cmdLine)
{
    try
    {
        char buffer[256];
        // Doc mot dong ket thuc bang ky tu xuong dong (\r\n)
        int bytesRead = slave.recvLine(buffer, sizeof(buffer));
        if (bytesRead >= 2)
        {
            // Cat bo ky tu \r\n o cuoi xau
            if (buffer[bytesRead - 1] == '\n') buffer[bytesRead - 1] = '\0';
            if (bytesRead >= 2 && buffer[bytesRead - 2] == '\r') buffer[bytesRead - 2] = '\0';
            cmdLine = buffer;
        }
        else if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            cmdLine = buffer;
        }
        return bytesRead;
    }
    catch (SocketException& e)
    {
        cerr << "[FTP Server] Loi doc lenh: " << e.what() << endl;
        return -1;
    }
}

unsigned short FTPServer::parseCmd(const string& cmdLine, string cmd_argv[], int& cmd_argc)
{
    cmd_argc = 0;
    istringstream iss(cmdLine);

    // Tach dong lenh thanh cac tu (argument) qua khoang trang
    while (cmd_argc < SERVER_CMD_ARG_NUM && (iss >> cmd_argv[cmd_argc]))
    {
        cmd_argc++;
    }

    // Tim kiem xem tu dau tien (ten lenh) co trong bang lenh hay khong
    if (cmd_argc > 0 && !cmd_argv[0].empty())
    {
        // Chuyen ten lenh thanh chu hoa de ho tro ca chu thuong va chu hoa
        string cmdUpper = cmd_argv[0];
        for (size_t i = 0; i < cmdUpper.size(); ++i)
        {
            cmdUpper[i] = toupper(cmdUpper[i]);
        }
        cmd_argv[0] = cmdUpper;

        for (int i = 0; i < this->numCmd; i++)
        {
            if (cmdUpper == cmdNameList[i])
            {
                return i; // Tra ve ID cua lenh tim thay
            }
        }
    }
    return SERVER_CMD_UNKNOWN;
}

void FTPServer::startNewSession(TcpSocket slave)
{
    // Tao mot phien lam viec moi (FTPSession) cho ket noi tu Client
    FTPSession* session = new FTPSession(slave, conf);
    string cmdLine;
    string cmdArgv[SERVER_CMD_ARG_NUM];
    int cmdArgc = 0;
    unsigned short cmdId = 0;

    try
    {
        // 1. Gui thong bao chao mung khi Client vua ket noi thanh cong
        slave.send("220 Welcome to Simple FTP Server v5 (Bulk Download Ready)\r\n");

        // 2. Vong lap nhan va thuc thi lenh cho den khi phien ket thuc (QUIT)
        while (!session->isQuit())
        {
            int cmdLen = readCmd(slave, cmdLine);
            if (cmdLen <= 0)
            {
                // Client da dong ket noi dot ngot
                break;
            }

            if (cmdLine.empty()) continue;

            // 3. Phan tich cu phap dong lenh
            cmdId = parseCmd(cmdLine, cmdArgv, cmdArgc);

            // 4. Goi ham xu ly tuong ung trong FTPSession
            doCmd(session, cmdId, cmdArgv, cmdArgc);
        }

        // 5. Giai phong phien lam viec khi hoan tat
        delete session;
    }
    catch (SocketException& e)
    {
        cerr << "[FTP Server] Ngoai le phien lam viec: " << e.what() << endl;
        delete session;
    }
}
