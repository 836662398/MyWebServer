//
// Created by rui836662398 on 2021/3/2.
//

#include "tcp_server.h"

#include <Net/tcp_connection.h>

std::string unit_name = "TcpServer";

TcpServer::TcpServer(EventLoop *loop, const SockAddress &listen_addr,
                     int thread_num, const std::string &name, bool is_reuseport)
    : loop_(loop),
      listen_addr_(listen_addr),
      ip_port_(listen_addr_.IpPort()),
      name_(name),
      acceptor_(std::make_unique<Acceptor>(loop, listen_addr_, is_reuseport)),
      thread_pool_(
          std::make_unique<EventLoopThreadPool>(loop, thread_num, name)),
      connection_callback_(TcpConnection::DefaultConnCallback),
      message_callback_(TcpConnection::DefaultMessageCallback),
      started_(false),
      next_conn_id_(1) {
    using namespace std::placeholders;
    assert(loop != nullptr);
    acceptor_->set_new_connection_callback(
        std::bind(&TcpServer::HandleNewConn, this, _1, _2));
}

TcpServer::TcpServer(EventLoop *loop, uint16_t port, int thread_num,
                     const std::string &name, bool is_reuseport)
    : TcpServer(loop, SockAddress(port), thread_num, name, is_reuseport) {}

TcpServer::~TcpServer() {
    loop_->AssertInLoopThread();
    TRACE(fmt::format("{} destructing", name_));
    for (auto &item : connections_) {
        TcpConnectionPtr conn(item.second);
        conn->loop()->RunInLoop(std::bind(&TcpConnection::ConnDestroy, conn));
    }
}

void TcpServer::StartListening() {
    bool b = false;
    // first para shouble be lvalue
    if (started_.compare_exchange_strong(b, true)) {
        thread_pool_->Start();

        assert(!acceptor_->IsListening());
        loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
    }
}

void TcpServer::HandleNewConn(int sockfd, const SockAddress &peer_addr) {
    loop_->AssertInLoopThread();
    EventLoop *io_loop = thread_pool_->GetNextLoop();
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", ip_port_.c_str(), next_conn_id_);
    ++next_conn_id_;
    std::string conn_name = name_ + buf;
    INFO(fmt::format("HandleNewConn(): new connection [{}] from {}", conn_name,
                     peer_addr.IpPort()));

    SockAddress local_addr(sockfd);
    TcpConnectionPtr conn(std::make_shared<TcpConnection>(
        io_loop, conn_name, sockfd, local_addr, peer_addr));
    connections_[conn_name] = conn;
    conn->set_connection_callback(connection_callback_);
    conn->set_message_callback(message_callback_);
    conn->set_write_complete_callback(write_complete_callback_);
    conn->set_close_callback(std::bind(
        &TcpServer::RemoveConn, this, std::placeholders::_1));
    io_loop->RunInLoop(std::bind(&TcpConnection::ConnEstablished, conn));
}

void TcpServer::RemoveConn(const TcpConnectionPtr &conn) {
    loop_->RunInLoop(std::bind(&TcpServer::RemoveConnInLoop, this, conn));
}
void TcpServer::RemoveConnInLoop(const TcpConnectionPtr &conn) {
    loop_->AssertInLoopThread();
    TRACE(fmt::format("RemoveConnInLoop() {} - connection {}", name_,
                      conn->name()));
    size_t n = connections_.erase(conn->name());
    (void)n;
    assert(n == 1);
    conn->loop()->RunInLoop(std::bind(&TcpConnection::ConnDestroy, conn));
}
