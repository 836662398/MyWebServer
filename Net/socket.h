//
// Created by rui836662398 on 2021/2/19.
//

#ifndef MYWEBSERVER_SOCKET_H
#define MYWEBSERVER_SOCKET_H

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
    void bind(const SockAddress& localaddr);
    // abort if address is in use
    void listen();
    // the accepted socketfd is returned and *peeraddr is assigned
    // if it succeeds. Otherwise, -1 is returned.
    int accept(SockAddress* peeraddr);

    void shutdownWrite();

    // Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
    void setTcpNoDelay(bool on);
    // Enable/disable SO_REUSEADDR
    void setReuseAddr(bool on);
    // Enable/disable SO_REUSEPORT
    void setReusePort(bool on);
    // Enable/disable SO_KEEPALIVE
    void setKeepAlive(bool on);

    static int getSocketError(int sockfd);

   private:
    int accept(int sockfd, struct sockaddr_in6* addr);

    int sockfd_;
};

#endif  // MYWEBSERVER_SOCKET_H
