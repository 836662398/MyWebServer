//
// Created by rui836662398 on 2021/3/4.
//

#ifndef MYWEBSERVER_TCP_CLIENT_H
#define MYWEBSERVER_TCP_CLIENT_H

#include <mutex>

#include "Net/connector.h"
#include "Net/tcp_connection.h"

class TcpClient : noncopyable {
   public:
    TcpClient(EventLoop* loop, const SockAddress& server_addr,
              const std::string& name);
    ~TcpClient();

    void Connect();
    void Disconnect();
    // Stop retrying Connect() instead of Disconnect
    void Stop();

    TcpConnectionPtr connection() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_;
    }

    EventLoop* loop() const { return loop_; }
    bool can_reconnect() const { return can_reconnect_; }
    // reconnect automatically if connection break
    void EnableReconnect() { can_reconnect_ = true; }

    const std::string& name() const { return name_; }

    // Not thread safe, must call in loop
    void set_connection_callback(ConnectionCallback cb) {
        connection_callback_ = std::move(cb);
    }
    // Not thread safe, must call in loop
    void set_message_callback(MessageCallback cb) {
        message_callback_ = std::move(cb);
    }
    // Not thread safe, must call in loop
    void set_write_complete_callback(WriteCompleteCallback cb) {
        write_complete_callback_ = std::move(cb);
    }

   private:
    using ConnectorPtr = std::shared_ptr<Connector>;

    // Not thread safe, but in loop
    void HandleNewConn(int sockfd);
    // Not thread safe, but in loop
    void RemoveConn(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    ConnectorPtr connector_;
    const std::string name_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    // reconnect automatically if connection break
    bool can_reconnect_;
    // for judge whether to reconnect
    bool is_connecting_;
    // always in loop thread
    int next_conn_id_;
    mutable std::mutex mutex_;
    TcpConnectionPtr connection_;  // GUARDED_BY mutex_
};

#endif  // MYWEBSERVER_TCP_CLIENT_H
