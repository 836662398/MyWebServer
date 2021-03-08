//
// Created by rui836662398 on 2021/2/9.
//
#include "Net/sock_addr.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace std;
TEST(SockAddressTest, test1) {
    SockAddress addr0(1234);
    EXPECT_EQ(addr0.Ip(), string("0.0.0.0"));
    EXPECT_EQ(addr0.IpPort(), string("0.0.0.0:1234"));
    EXPECT_EQ(addr0.Port(), 1234);

    addr0 = SockAddress(1234, "::1", true);
    EXPECT_EQ(addr0.Ip(), string("::1"));
    EXPECT_EQ(addr0.IpPort(), string("::1:1234"));
    EXPECT_EQ(addr0.Port(), 1234);

    addr0 = SockAddress(1234, "::", true);
    EXPECT_EQ(addr0.Ip(), string("::"));
    EXPECT_EQ(addr0.IpPort(), string(":::1234"));
    EXPECT_EQ(addr0.Port(), 1234);

    addr0 = SockAddress(1234, "0:0:0:0:0:0:0:0", true);
    EXPECT_EQ(addr0.Ip(), string("::"));
    EXPECT_EQ(addr0.IpPort(), string(":::1234"));
    EXPECT_EQ(addr0.Port(), 1234);

    addr0 = SockAddress(55555, "1:2:3:4:5:6:7:8", true);
    EXPECT_EQ(addr0.Ip(), string("1:2:3:4:5:6:7:8"));
    EXPECT_EQ(addr0.IpPort(), string("1:2:3:4:5:6:7:8:55555"));
    EXPECT_EQ(addr0.Port(), 55555);

    SockAddress addr1(4321, "192.168.1.1");
    EXPECT_EQ(addr1.Ip(), string("192.168.1.1"));
    EXPECT_EQ(addr1.IpPort(), string("192.168.1.1:4321"));
    EXPECT_EQ(addr1.Port(), 4321);

    SockAddress addr4 = SockAddress("192.168.1.1", 4321);
    EXPECT_EQ(addr4.Ip(), string("192.168.1.1"));
    EXPECT_EQ(addr4.IpPort(), string("192.168.1.1:4321"));
    EXPECT_EQ(addr4.Port(), 4321);

    SockAddress addr2("1.2.3.4", 0);
    EXPECT_EQ(addr2.Ip(), string("1.2.3.4"));
    EXPECT_EQ(addr2.IpPort(), string("1.2.3.4:0"));
    EXPECT_EQ(addr2.Port(), 0);

    SockAddress addr3("255.254.253.252", 65535);
    EXPECT_EQ(addr3.Ip(), string("255.254.253.252"));
    EXPECT_EQ(addr3.IpPort(), string("255.254.253.252:65535"));
    EXPECT_EQ(addr3.Port(), 65535);
}

TEST(SockAddressTest, test2) {
    SockAddress addr(80);
    EXPECT_TRUE(SockAddress::Resolve("baidu.com", &addr));
    cout << "baidu.com resolved to " << addr.IpPort()<<endl;
}