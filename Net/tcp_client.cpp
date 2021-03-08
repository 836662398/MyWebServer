//
// Created by rui836662398 on 2021/3/4.
//

#include "tcp_client.h"

#include "Net/event_loop.h"
#include "Utility/logging.h"

std::string unit_name = "TcpClient";

TcpClient::TcpClient(EventLoop* loop, const SockAddress& serverAddr,
                     const std::string& name)
    : loop_(loop),
      connector_(std::make_shared<Connector>(loop, serverAddr)),
      name_(name),
      connection_callback_(TcpConnection::DefaultConnCallback),
      message_callback_(TcpConnection::DefaultMessageCallback),
      can_reconnect_(false),
      is_connecting_(false),
      next_conn_id_(1) {
    assert(loop != nullptr);
    connector_->setNewConnectionCallback(
        std::bind(&TcpClient::HandleNewConn, this, std::placeholders::_1));
    INFO(fmt::format("{} ctor ", name_));
}

TcpClient::~TcpClient() {
    INFO(fmt::format("{} dtor ", name_));
    TcpConnectionPtr conn;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        conn = connection_;
    }
    if (conn) {
        assert(loop_ == conn->loop());
        conn->Close();
    } else {
        connector_->Stop();
    }
}

void TcpClient::Connect() {
    INFO(fmt::format("Connect() {} - connecting to {}", name_,
                     connector_->server_address().IpPort()));
    is_connecting_ = true;
    connector_->Start();
}

void TcpClient::Disconnect() {
    is_connecting_ = false;
    {
        std::lock_guard<std::mutex> lck(mutex_);
        if (connection_) {
            connection_->Close();
        }
    }
}

void TcpClient::Stop() {
    is_connecting_ = false;
    connector_->Stop();
}

void TcpClient::HandleNewConn(int sockfd) {
    loop_->AssertInLoopThread();
    SockAddress peer_addr(Socket::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peer_addr.IpPort().c_str(),
             next_conn_id_);
    ++next_conn_id_;
    std::string conn_name = name_ + buf;

    INFO(fmt::format("HandleNewConn(): new connection [{}] ", conn_name));

    SockAddress local_addr = SockAddress::CreateSockAddressByFd(sockfd);

    TcpConnectionPtr conn(std::make_shared<TcpConnection>(
        loop_, conn_name, sockfd, local_addr, peer_addr));

    conn->set_connection_callback(connection_callback_);
    conn->set_message_callback(message_callback_);
    conn->set_write_complete_callback(write_complete_callback_);
    conn->set_close_callback(
        std::bind(&TcpClient::RemoveConn, this, std::placeholders::_1));
    {
        std::lock_guard<std::mutex> lck(mutex_);
        connection_ = conn;
    }
    conn->ConnEstablished();
}

void TcpClient::RemoveConn(const TcpConnectionPtr& conn) {
    loop_->AssertInLoopThread();
    assert(loop_ == conn->loop());

    {
        std::lock_guard<std::mutex> lck(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->QueueInLoop(std::bind(&TcpConnection::ConnDestroy, conn));
    if (can_reconnect_ && is_connecting_) {
        INFO(fmt::format("{} - Reconnecting to {}", name_,
                         connector_->server_address().IpPort()));
        connector_->Restart();
    }
}
