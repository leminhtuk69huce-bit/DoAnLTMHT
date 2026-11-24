#ifndef __TCP_SERVER_SOCKET_H__
#define __TCP_SERVER_SOCKET_H__

#include "tcpsocket.h"


/**
 *   Socket which is able to bind, listen, and accept connection
 */
class TcpServerSocket : public Socket
{
public:
    /**
     *   accept new connection and return a TcpSocket to send/recv data
     */
    TcpSocket accept(long timeout = 0);
    void close();
    void setListen(unsigned short port, int queueLength = 10);
    bool isListening();
    /**
     *   Read into the given buffer up to bufferLen bytes data from this
     *   socket.  Call connect() before calling recv()
     */
public:
    TcpServerSocket();
    ~TcpServerSocket();
    TcpServerSocket(const string &localAddress, unsigned short port, int queueLength=10);
    TcpServerSocket(unsigned short port, int queueLength = 10);
    //TcpServerSocket(int newConnSD);
private:
    /**
     *   Listen on a port
     */
    void listen(int queueLength = 10);
};

#endif

