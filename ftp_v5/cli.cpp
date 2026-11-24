#include <iostream>
#include <sstream>
#include "cli.h"


CmdLineInterface::CmdLineInterface()
{
    numCommands = 0;
    cmdDefaulID = 0;
    cmdPrompt ="cli>";
    bRunning = false;
}

CmdLineInterface::CmdLineInterface(const string& prompt)
{
    numCommands = 0;
    cmdPrompt = prompt;
    bRunning = false;
}

// Xoa man hinh
void CmdLineInterface::clearScreen()
{
#ifdef __linux__
    system("clear");
#elif _WIN32
    system("cls");
#endif // __linux__
}

void CmdLineInterface::setCmdPrompt(const string& prompt)
{
    this->cmdPrompt = prompt;
}

// Hien thi dau nhac lenh
void CmdLineInterface::showCmdPrompt()
{
    cout << this->cmdPrompt ;
}

void CmdLineInterface::addCmd(const string& name, CLI_CMD_FUNC f)
{
    this->cmdDoFunc[this->numCommands]=f;
    cmdNameList[this->numCommands] = name;
    this->numCommands++;
}

void CmdLineInterface::initCmd()
{
//
}

void CmdLineInterface::initConsole()
{
//
}

// Doc lenh nhap tu ban phim
void CmdLineInterface::readCmd(string& cmd)
{
    if (!getline(cin, cmd))
    {
        this->bRunning = false;
        cmd = "";
    }
}

// Ham phan tich cu phap

unsigned short CmdLineInterface::parseCmd(const string& cmd, string cmd_argv[], int& cmd_argc)
{
    // phan tich xau ki tu nguoi dung nhap vao de xac dinh lenh va tham so

    // Cu phap:  <ten lenh>space<tham so 1>space<tham so 2>
    //           Moi thanh phan cua lenh cach nhau 1 hoac nhieu dau space

    cmd_argc = 0; // so tham so = 0

    if(!cmd.empty())
    {
        istringstream iss(cmd);
        while(iss.good() && cmd_argc < CMD_MAX_ARG_NUM)
        {
            iss >> cmd_argv[cmd_argc++];
        }
    }

    if(cmd_argc>0)
    {
        for(unsigned short i = 0; i<this->numCommands; i++)
        {
            if(cmd_argv[0]==cmdNameList[i])
                return i;
        }
        if(cmd_argv[0]==CMD_QUIT_NAME)
            return CMD_QUIT_ID;
    }
    else
        return CMD_NOOP_ID;
    return CMD_UNKNOWN_ID;
}

// ham thuc hien khi lenh khong dung "unknown"
void CmdLineInterface::doUnknown()
{
    cout << "Lenh khong duoc ho tro\nGo lenh help de duoc tro giup\n";
}

// ham thuc hien lenh "quit"
void CmdLineInterface::doQuit()
{
    this->bRunning = false;
    cout << "Chuong trinh ket thuc\n";
}

void CmdLineInterface::doCmd(unsigned short id, string cmd_argv[], int cmd_argc)
{
    if(id==CMD_NOOP_ID)
    {
        return;
    }
    else if(id==CMD_UNKNOWN_ID)
    {
        doUnknown();
    }
    else if(id==CMD_QUIT_ID)
    {
        doQuit();
    }
    else
    {
        CLI_CMD_FUNC f = this->cmdDoFunc[id];
        if(f!=NULL)
            (this->*f)(cmd_argv, cmd_argc);
    }
}

void CmdLineInterface::run(char* initArgv[], int initArgc)
{
    string cmd;  // command string
    string cmdArgArray[CMD_MAX_ARG_NUM]; // arguments
    int cmdArgCount;
    unsigned short cid;  // command id
    bRunning = true;

    // run initial command with its argument
    if(initArgc > 1 && numCommands > cmdDefaulID)
    {
        for(int i = 0; i<initArgc;++i)
        {
            cmdArgArray[i] = initArgv[i];
        }
        cmdArgCount = initArgc;
        doCmd(cmdDefaulID,cmdArgArray,cmdArgCount);
    }

    while (bRunning) // chu trinh chinh - MAIN LOOP
    {
        // hien thi dau nhac lenh
        showCmdPrompt();
        // nhap lenh
        readCmd(cmd);
        // phan tich lenh
        cid = parseCmd(cmd, cmdArgArray, cmdArgCount);
        // thuc hien lenh va hien thi ket qua
        doCmd(cid, cmdArgArray, cmdArgCount);
    }
}


void CmdLineInterface::run()
{
    string cmd;  // command string
    string cmdArgArray[CMD_MAX_ARG_NUM]; // arguments
    int cmdArgCount;
    unsigned short cid;  // command id
    bRunning = true;
    while (bRunning) // chu trinh chinh - MAIN LOOP
    {
        // hien thi dau nhac lenh
        showCmdPrompt();
        // nhap lenh
        readCmd(cmd);
        // phan tich lenh
        cid = parseCmd(cmd, cmdArgArray, cmdArgCount);
        // thuc hien lenh va hien thi ket qua
        doCmd(cid, cmdArgArray, cmdArgCount);
    }
}

