#include "session.h"

Session::Session(const TcpSocket& slave, ServerConfig* conf)
{
    this->slave = slave;
    this->quitSession = false;
    this->conf = conf;
}
Session::~Session()
{
    if(slave.isAlive())
        slave.close();
}



void Session::reset()
{
}


bool Session::isQuit()
{
    return quitSession;
}


