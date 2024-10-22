//
// Created by rui836662398 on 2021/2/19.
//

#ifndef MYWEBSERVER_SOCKET_H
#define MYWEBSERVER_SOCKET_H

#include <sys/socket.h>
#include <unistd.h>

#include "Utility/noncopyable.h"

class SockAddress;

// Wrapper of socket file descriptor.
class Socket : noncopyable {
   public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();

    int fd() const { return sockfd_; }

    // abort if address is in use
    void Bind(const SockAddress& localaddr);
    // abort if address is in use
    void Listen();
    // the accepted socketfd is returned and *peeraddr is assigned
    // if it succeeds. Otherwise, -1 is returned.
    int Accept(SockAddress* peeraddr);

    void ShutdownWrite();

    // Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
    void setTcpNoDelay(bool on);
    // Enable/disable SO_REUSEADDR
    void setReuseAddr(bool on);
    // Enable/disable SO_REUSEPORT
    void setReusePort(bool on);
    // Enable/disable SO_KEEPALIVE
    void setKeepAlive(bool on);

    static int CreateSocketFd(sa_family_t family);

   public:
    // utility about sockfd
    static void close(int sockfd);

    static int getSocketError(int sockfd);

    static struct sockaddr_in6 getLocalAddr(int sockfd);
    static struct sockaddr_in6 getPeerAddr(int sockfd);
    static bool IsSelfConnect(int sockfd);


   private:
    int accept(int sockfd, struct sockaddr_in6* addr);

    int sockfd_;
};

#endif  // MYWEBSERVER_SOCKET_H
