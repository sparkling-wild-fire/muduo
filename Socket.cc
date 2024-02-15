#include "Socket.h"
#include "Logger.h"
#include "InetAddress.h"

#include <unistd.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

Socket::~Socket()
{
    close(sockfd_);
}

void Socket::bindAddress(const InetAddress &localaddr)
{
    if (0 != ::bind(sockfd_, (sockaddr*)localaddr.getSockAddr(), sizeof(sockaddr_in)))   // ip+port
    {
        LOG_FATAL("bind sockfd:%d fail \n", sockfd_);
    }
}

// 监听连接
void Socket::listen()
{
    // 第二个参数是在连接队列中等待的连接数量的最大值（通常被称为backlog）。
    // 在调用listen()函数之后，套接字将变为被动模式，等待客户端连接。一旦有客户端连接请求到达，服务器将接受该连接并返回一个新的套接字描述符，可以使用该描述符与客户端通信
    if (0 != ::listen(sockfd_, 1024))
    {
        LOG_FATAL("listen sockfd:%d fail \n", sockfd_);
    }
}

// 创建fd并返回
int Socket::accept(InetAddress *peeraddr)
{
    /**
     * 1. accept函数的参数不合法
     * 2. 对返回的connfd没有设置非阻塞
     * Reactor模型 one loop per thread
     * poller + non-blocking IO
     */ 
    sockaddr_in addr;  // 不用支持ipv6   联合体
    socklen_t len = sizeof addr;
    bzero(&addr, sizeof addr);   // 将地址清0
    // sockfd 是要接收连接请求的监听套接字的文件描述符；addr 和 addrlen 分别是指向存储客户端地址的结构体和结构体大小的指针；flags 是一个用于设置标志位的整数。
    // accept4函数的第一个参数addr是用来存储接受到的客户端地址信息的。它是一个指向sockaddr结构体的指针，该结构体用于存储网络地址信息，包括IP地址和端口号等。
    // 在调用accept4函数之前，我们需要先创建一个套接字，并将其绑定到一个本地地址上。当客户端连接到服务器时，accept4函数会接受这个连接，并将客户端的地址信息存储到addr参数所指向的内存中。
    //   =》 怎么读到客户端地址的呢？   这个函数什么时候被调用的？
    int connfd = ::accept4(sockfd_, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0)
    {
        peeraddr->setSockAddr(addr);   // 通过入参把客户端地址返回回去
    }
    return connfd;   // 把fd返回回去
}

void Socket::shutdownWrite()
{
    if (::shutdown(sockfd_, SHUT_WR) < 0)
    {
        LOG_ERROR("shutdownWrite error");
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}

void Socket::setReuseAddr(bool on)  // 地址复用什么意思
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}