//
// Created by rui836662398 on 2021/3/5.
//

#include <spdlog/spdlog.h>

#include "Net/tcp_server.h"

class EchoServer {
   public:
    EchoServer(EventLoop* loop, uint16_t port, int loop_num)
        : loop_(loop), server_(loop, port, loop_num, "EchoServer") {
        using namespace std::placeholders;
        server_.set_connection_callback(
            std::bind(&EchoServer::onConnection, this, _1));
        server_.set_message_callback(
            std::bind(&EchoServer::onMessage, this, _1, _2));
    }

    void Start() { server_.StartListening();
        spdlog::info(fmt::format("EchoServer listening in {}.", server_.ip_port()));
    }

   private:
    void onConnection(const TcpConnectionPtr& conn) {
        spdlog::info(fmt::format("{} -> {} is {}", conn->peer_addr().IpPort(),
                                 conn->local_addr().IpPort(),
                                 (conn->Connected() ? "UP" : "DOWN")));
        conn->Send("Hello");
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf) {
        std::string msg(buf->TakeOutAll());
        spdlog::info(fmt::format("[{}] recv {} bytes: {}", conn->name(), msg.size(), msg));
        if (msg == "exit") {
            conn->Send("bye");
            conn->ForceClose();
            printf("ForceClose()\n");
        } else if (msg == "quit") {
            loop_->Quit();
        } else
            conn->Send(msg);
    }

    EventLoop* loop_;
    TcpServer server_;
};

int main(){
    EventLoop loop;
    EchoServer server(&loop, 8888, 2);
    server.Start();
    loop.Loop();
}