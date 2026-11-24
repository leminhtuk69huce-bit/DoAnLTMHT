#ifndef SESSION_H_
#define SESSION_H_
#include "tcpsocket.h"
#include "serverconfig.h"



#define SERVER_CMD_MAX_NUM 64
#define SERVER_CMD_ARG_LEN 64
#define SERVER_CMD_ARG_NUM 16
#define SERVER_CMD_BUF_LEN 256

#define SERVER_CMD_UNKNOWN  SERVER_CMD_MAX_NUM + 1


class Session
{
protected:
    ServerConfig* conf;
    TcpSocket slave;
    bool quitSession;
private:

public:
    Session(const TcpSocket& slave,ServerConfig* conf);
    virtual ~Session();
    virtual void doUnknown(string cmd_argv[], int cmd_argc)=0;
    virtual void reset();
    bool isQuit();
};

typedef void (Session::*CMD_FUNC)(string [], int);
#define FUNC_CAST(p_func_type) static_cast<CMD_FUNC>(p_func_type)

#endif // SESSION_H_
