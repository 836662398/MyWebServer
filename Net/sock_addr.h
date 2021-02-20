//
// Created by rui836662398 on 2021/2/8.
//

#ifndef MYWEBSERVER_SOCK_ADDR_H
#define MYWEBSERVER_SOCK_ADDR_H

#include <netinet/in.h>

#include <string_view>

// Wrapper of sockaddr_in.
class SockAddress {
   public:
    SockAddress() {}
    // Constructs an endpoint with given ip and port.
    // ip should be "1.2.3.4"
    // for Server
    explicit SockAddress(uint16_t port, std::string_view ip = "0.0.0.0",
                         bool is_ipv6 = false);
    // for Client
    explicit SockAddress(std::string_view ip, uint16_t port = 0,
                         bool is_ipv6 = false);
    explicit SockAddress(const struct sockaddr_in& addr) : addr_(addr) {}
    explicit SockAddress(const struct sockaddr_in6& addr) : addr6_(addr) {}

    std::string Ip() const;
    std::string IpPort() const;
    uint16_t Port() const { return be16toh(PortNetEndian()); }
    sa_family_t family() const { return addr_.sin_family; }
    uint16_t PortNetEndian() const { return addr_.sin_port; }

    const sockaddr_in* get_sockaddr_in() const { return &addr_; }
    const sockaddr_in6* get_sockaddr_in6() const { return &addr6_; }
    void set_sockaddr_in6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

    // some utilities of sockaddr
    // protocol-dependent
    static void IpPortToSockaddr(const char* ip, uint16_t port,
                                 struct sockaddr_in* addr);
    static void IpPortToSockaddr(const char* ip, uint16_t port,
                                 struct sockaddr_in6* addr);
    // protocol-independent
    static void SockaddrtoIpPort(char* buf, size_t size,
                                 const struct sockaddr* addr);
    static void SockaddrtoIp(char* buf, size_t size,
                             const struct sockaddr* addr);
    // resolve hostname to IPv4 address, not changing port or sin_family
    // return true on success.
    // thread safe
    static bool Resolve(std::string_view hostname, SockAddress* out);

   private:
    void CtorImpl(uint16_t port, std::string_view ip, bool is_ipv6);

    union {
        sockaddr_in addr_;
        sockaddr_in6 addr6_;
    };
};

#endif  // MYWEBSERVER_SOCK_ADDR_H
