//
// Created by rui836662398 on 2021/3/5.
//

#include "Net/tcp_client.h"

#include <gtest/gtest.h>

#include "Net/event_loop.h"
#include "Net/event_loop_thread.h"
#include "Utility/logging.h"

using namespace std;

TcpClient* g_client;

TEST(TcpClient, test) {
    EventLoopThread t;
    EventLoop* ploop = t.StartLoop();

    SockAddress serverAddr("180.101.49.11", 80);
    TcpClient client(ploop, serverAddr, "TcpClient");
    g_client = &client;
    client.Connect();
    sleep(2);
}

void Timeout() {
    spdlog::info("Timeout()");
    g_client->Stop();
}

TEST(TcpClient, syn) {
    EventLoop loop;

    SockAddress serverAddr("127.0.0.1", 2);  // no such server
    TcpClient client(&loop, serverAddr, "TcpClient");
    g_client = &client;
    client.Connect();
    loop.RunAfter(0.0, Timeout);
    loop.RunAfter(1, [ploop = &loop] { ploop->Quit(); });
    loop.Loop();
}

TEST(TcpClient, asyn) {
    EventLoopThread t;
    EventLoop* ploop = t.StartLoop();

    SockAddress serverAddr("127.0.0.1", 2);
    TcpClient client(ploop, serverAddr, "TcpClient");
    g_client = &client;
    client.Connect();
    ploop->RunAfter(0.0, Timeout);
    sleep(2);
}

