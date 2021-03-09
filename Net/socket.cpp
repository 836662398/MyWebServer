//
// Created by rui836662398 on 2021/2/19.
//

#include "socket.h"

#include <netinet/tcp.h>
#include <sys/types.h>

#include "Net/sock_addr.h"
#include "Utility/logging.h"
#include "Utility/types.h"

// for logging
static std::string unit_name = "Socket";

Socket::~Socket() { close(sockfd_); }

void Socket::Bind(const SockAddress& localaddr) {
    int ret =
        ::bind(sockfd_,
               reinterpret_cast<const sockaddr*>(localaddr.get_sockaddr_in6()),
               static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        FATAL(fmt::format("Socket failed to bind. {}.", strerror_tl(errno)));
    }
}

void Socket::Listen() {
    int ret = ::listen(sockfd_, SOMAXCONN);
    if (ret < 0) {
        FATAL("Socket failed to listen.");
    }
}

int Socket::accept(SockAddress* peeraddr) {
    struct sockaddr_in6 addr;
    int connfd = accept(sockfd_, &addr);
    if (connfd >= 0) {
        peeraddr->set_sockaddr_in6(addr);
    }
    return connfd;
}

void Socket::shutdownWrite() {
    if (shutdown(sockfd_, SHUT_WR) < 0) {
        ERROR("Socket failed to shutdownWrite.");
    }
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval,
                           static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on) {
        ERROR("TCP_NODELAY failed.");
    }
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval,
                           static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on) {
        ERROR("SO_REUSEADDR failed.");
    }
}

// multiple processes can share ports for load balancing
void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval,
                           static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on) {
        ERROR("SO_REUSEPORT failed.");
    }
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval,
                           static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on) {
        ERROR("SO_KEEPALIVE failed.");
    }
}

int Socket::getSocketError(int sockfd) {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        // getsockopt failed
        return errno;
    } else {
        return optval;
    }
}

int Socket::CreateSocketFd(sa_family_t family) {
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                          IPPROTO_TCP);
    if (sockfd < 0) {
        FATAL(fmt::format("socket() failed! error {}", strerror_tl(errno)));
    }
}

void Socket::close(int sockfd) {
    if (::close(sockfd) < 0) {
        ERROR("close() failed!");
    }
}

struct sockaddr_in6 Socket::getLocalAddr(int sockfd) {
    struct sockaddr_in6 localaddr;
    memzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&localaddr),
                      &addrlen) < 0) {
        ERROR("getLocalAddr() failed!");
    }
    return localaddr;
}

struct sockaddr_in6 Socket::getPeerAddr(int sockfd) {
    struct sockaddr_in6 peeraddr;
    memzero(&peeraddr, sizeof peeraddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername(sockfd, reinterpret_cast<struct sockaddr*>(&peeraddr),
                      &addrlen) < 0) {
        ERROR("getPeerAddr() failed!");
    }
    return peeraddr;
}

// selfconnect: https://gameinstitute.qq.com/community/detail/112599
bool Socket::IsSelfConnect(int sockfd) {
    struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
    struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
    if (localaddr.sin6_family == AF_INET) {
        const struct sockaddr_in* laddr4 =
            reinterpret_cast<struct sockaddr_in*>(&localaddr);
        const struct sockaddr_in* raddr4 =
            reinterpret_cast<struct sockaddr_in*>(&peeraddr);
        return laddr4->sin_port == raddr4->sin_port &&
               laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
    } else if (localaddr.sin6_family == AF_INET6) {
        return localaddr.sin6_port == peeraddr.sin6_port &&
               memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr,
                      sizeof localaddr.sin6_addr) == 0;
    } else {
        return false;
    }
}

int Socket::accept(int sockfd, struct sockaddr_in6* addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
    int connfd = ::accept4(sockfd, reinterpret_cast<sockaddr*>(addr), &addrlen,
                           SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0) {
        int savedErrno = errno;
        //        ERROR("Socket failed to accept");
        switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPERM:
            case EMFILE:  // per-process lmit of open file desctiptor
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                ERROR(
                    fmt::format("unexpected error of ::accept {}", savedErrno));
                break;
            default:
                ERROR(fmt::format("unknown error of ::accept {}", savedErrno));
                break;
        }
    }
    return connfd;
}