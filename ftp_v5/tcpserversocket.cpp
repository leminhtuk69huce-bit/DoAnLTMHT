#ifdef _WIN32
#include <winsock2.h>         // For socket(), connect(), send(), and recv()
#include <ws2tcpip.h>
typedef int socklen_t;
typedef char raw_type;       // Type used for raw data on this platform
#else
#include <sys/types.h>       // For data types
#include <sys/socket.h>      // For socket(), connect(), send(), and recv()
#include <netdb.h>           // For gethostbyname()
#include <arpa/inet.h>       // For inet_addr()
#include <unistd.h>          // For close()
#include <netinet/in.h>      // For sockaddr_in
typedef void raw_type;       // Type used for raw data on this platform
#endif

#include <errno.h>             // For errno
#include "tcpserversocket.h"

//using namespace std;
TcpServerSocket::TcpServerSocket():Socket(SOCK_STREAM, IPPROTO_TCP)
{
#ifdef __linux__
    int reusePort = 1;
    setsockopt(sockDesc, SOL_SOCKET, SO_REUSEADDR, &reusePort, sizeof(reusePort));
    setsockopt(sockDesc, SOL_SOCKET, SO_REUSEPORT, &reusePort, sizeof(reusePort));
#endif // __linux__
}

TcpServerSocket::TcpServerSocket(unsigned short localPort, int queueLen):Socket(SOCK_STREAM, IPPROTO_TCP)
{
#ifdef __linux__
    int reusePort = 1;
    setsockopt(sockDesc, SOL_SOCKET, SO_REUSEADDR, &reusePort, sizeof(reusePort));
    setsockopt(sockDesc, SOL_SOCKET, SO_REUSEPORT, &reusePort, sizeof(reusePort));
#endif // __linux__
    setLocalPort(localPort);
    listen(queueLen);
}

TcpServerSocket::TcpServerSocket(const string &localAddress,
                                 unsigned short localPort, int queueLen): Socket(SOCK_STREAM, IPPROTO_TCP)
{
#ifdef __linux__
    int reusePort = 1;
    setsockopt(sockDesc, SOL_SOCKET, SO_REUSEADDR, &reusePort, sizeof(reusePort));
    setsockopt(sockDesc, SOL_SOCKET, SO_REUSEPORT, &reusePort, sizeof(reusePort));
#endif // __linux__
    setLocalAddressAndPort(localAddress, localPort);
    listen(queueLen);
}

TcpServerSocket::~TcpServerSocket()
{
#ifdef _WIN32
    if(sockDesc>0)
        ::closesocket(sockDesc);
#elif __linux__
    if(sockDesc>0)
    {
    ::shutdown(sockDesc,SHUT_RD);
    ::close(sockDesc);
    }
#endif
    sockDesc = -1;
}

void TcpServerSocket::setListen(unsigned short localPort, int queueLength)
{
    if(sockDesc==-1)
    {
        // Make a new socket
        if ((sockDesc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        {
            throw SocketException("Socket creation failed (socket())", true);
        }
    }
    setLocalPort(localPort);
    listen(queueLength);
}

bool TcpServerSocket::isListening()
{
#ifdef __linux__
    char val;
    if(sockDesc <= 0)
        return false;
    socklen_t len = sizeof(val);
    if (getsockopt(sockDesc, SOL_SOCKET, SO_ACCEPTCONN, &val, &len) == -1)
        return false;
    else
        return val ? true:false;
#elif _WIN32
    return (sockDesc > 0);
#endif // _WIN32
}

TcpSocket TcpServerSocket::accept(long timeout)
{
    int newConnSD;
    if(timeout==0)
    {
        if ((newConnSD = ::accept(sockDesc, NULL, 0)) < 0)
        {
            throw SocketException("Accept failed (accept())", true);
        }
    }
    else
    {
        fd_set	rfds;			/* read file descriptor set	*/
        FD_ZERO(&rfds);
        timeval tv = {timeout,0};
        FD_SET(sockDesc,&rfds);
        int ret;
        ret = select(sockDesc+1, &rfds, NULL, NULL,&tv);
        if (ret < 0) // Error
        {
            throw SocketException("Select failed (select())", true);
        }
        else if(ret > 0) // Has new connections
        {
            if (FD_ISSET(sockDesc, &rfds))
            {
                if ((newConnSD = ::accept(sockDesc, NULL, 0)) < 0)
                {
                    throw SocketException("Accept failed (accept())", true);
                }
            }
            else {
                throw SocketException("Accept failed (accept() socket error)", true);
            }

        } else { // timeout with no connection
            throw SocketException("Accept failed (accept() time out)", true);
        }
    }
    //return new TcpSocket(newConnSD);
    return TcpSocket(newConnSD);
}

void TcpServerSocket::listen(int queueLen)
{
    if (::listen(sockDesc, queueLen) < 0)
    {
        throw SocketException("Set listening socket failed (listen())", true);
    }
}

void TcpServerSocket::close()
{
#ifdef _WIN32
    if(sockDesc>0)
    {
        ::closesocket(sockDesc);
    }
#elif __linux__
    if(sockDesc>0)
    {
        ::shutdown(sockDesc,SHUT_RD);
        ::close(sockDesc);
    }
#endif
    sockDesc = -1;
}
