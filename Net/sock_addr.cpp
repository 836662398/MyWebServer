//
// Created by rui836662398 on 2021/2/8.
//

#include "sock_addr.h"

#include <arpa/inet.h>
#include <netdb.h>

#include <cstring>

#include "Utility/logging.h"

// for logging
static std::string unit_name = "SockAddress";

SockAddress::SockAddress(uint16_t port, std::string_view ip, bool is_ipv6) {
    CtorImpl(port, ip, is_ipv6);
}

SockAddress::SockAddress(std::string_view ip, uint16_t port, bool is_ipv6) {
    CtorImpl(port, ip, is_ipv6);
}

void SockAddress::CtorImpl(uint16_t port, std::string_view ip, bool is_ipv6) {
    if (is_ipv6) {
        memset(&addr6_, 0, sizeof addr6_);
        IpPortToSockaddr(ip.data(), port, &addr6_);
    } else {
        memset(&addr_, 0, sizeof addr_);
        IpPortToSockaddr(ip.data(), port, &addr_);
    }
}

void SockAddress::IpPortToSockaddr(const char *ip, uint16_t port,
                                   struct sockaddr_in *addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = htobe16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
        ERROR("inet_pton error!");
    }
}
void SockAddress::IpPortToSockaddr(const char *ip, uint16_t port,
                                   struct sockaddr_in6 *addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = htobe16(port);
    if (inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
        ERROR("inet_pton error!");
    }
}

void SockAddress::SockaddrtoIp(char *buf, size_t size,
                               const struct sockaddr *addr) {
    if (addr->sa_family == AF_INET) {
        assert(size >= INET_ADDRSTRLEN);
        auto addr4 = reinterpret_cast<const struct sockaddr_in *>(addr);
        inet_ntop(AF_INET, &addr4->sin_addr, buf, size);
    } else if (addr->sa_family == AF_INET6) {
        assert(size >= INET6_ADDRSTRLEN);
        auto addr6 = reinterpret_cast<const struct sockaddr_in6 *>(addr);
        inet_ntop(AF_INET6, &addr6->sin6_addr, buf, size);
    }
}

void SockAddress::SockaddrtoIpPort(char *buf, size_t size,
                                   const struct sockaddr *addr) {
    SockaddrtoIp(buf, size, addr);
    size_t end = strlen(buf);
    auto addr4 = reinterpret_cast<const struct sockaddr_in *>(addr);
    uint16_t port = be16toh(addr4->sin_port);
    assert(size >= end + 7);
    snprintf(buf + end, size - end, ":%u", port);
}

std::string SockAddress::Ip() const {
    char buf[64];
    SockaddrtoIp(buf, 64, reinterpret_cast<const struct sockaddr *>(&addr_));
    return buf;
}
std::string SockAddress::IpPort() const {
    char buf[64];
    SockaddrtoIpPort(buf, 64,
                     reinterpret_cast<const struct sockaddr *>(&addr_));
    return buf;
}

// for thread safety
static thread_local char t_resolveBuffer[8 * 1024];
bool SockAddress::Resolve(std::string_view hostname, SockAddress *out) {
    assert(out != NULL);
    struct hostent hent;
    struct hostent *he = NULL;
    int herrno = 0;
    memset(&hent, 0, sizeof hent);

    int ret = gethostbyname_r(hostname.data(), &hent, t_resolveBuffer,
                              sizeof t_resolveBuffer, &he, &herrno);
    if (ret == 0 && he != NULL) {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr *>(he->h_addr);
        return true;
    } else {
        if (ret) {
            ERROR("SockAddress::Resolve error!");
        }
        return false;
    }
}
