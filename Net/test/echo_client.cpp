//
// Created by rui836662398 on 2021/3/5.
//

#include <iostream>
#include <string>

#include <spdlog/spdlog.h>

#include "Net/event_loop_thread.h"
#include "Net/tcp_client.h"

class EchoClient {
   public:
    EchoClient(EventLoop* loop, const SockAddress& peer_addr)
        : loop_(loop), client_(loop, peer_addr, "EchoClient") {
        using namespace std::placeholders;
        client_.set_connection_callback(
            std::bind(&EchoClient::onConnection, this, _1));
        client_.set_message_callback(
            std::bind(&EchoClient::onMessage, this, _1, _2));
        client_.EnableReconnect();
    }

    void Connect() { client_.Connect(); }

    void Send(std::string_view msg){
        client_.connection()->Send(msg);
        if(msg == "exit"){
            client_.Disconnect();
        }
    }

   private:
    void onConnection(const TcpConnectionPtr& conn) {
        spdlog::info(fmt::format("{} -> {} is {}", conn->local_addr().IpPort(),
                                 conn->peer_addr().IpPort(),
                                 (conn->Connected() ? "UP" : "DOWN")));
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf) {
        std::string msg(buf->TakeOutAll());
        printf("recv msg: %s\n", msg.data());
    }

    EventLoop* loop_;
    TcpClient client_;
};

int main(){
    EventLoopThread t;
    EventLoop* ploop = t.StartLoop();
    SockAddress server_addr("127.0.0.1", 8888);
    EchoClient client(ploop, server_addr);
    client.Connect();
    std::string buf;
    while(getline(std::cin, buf, '\n')){
        client.Send(buf);
    }
}