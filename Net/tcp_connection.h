//
// Created by rui836662398 on 2021/3/1.
//

#ifndef MYWEBSERVER_TCP_CONNECTION_H
#define MYWEBSERVER_TCP_CONNECTION_H

#include <memory>

#include "Net/buffer.h"
#include "Net/callbacks.h"
#include "Net/sock_addr.h"
#include "Utility/noncopyable.h"

class Channel;
class EventLoop;
class Socket;

// wrapper of established connections
// Invisible to users
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
   public:
    TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                  const SockAddress& local, const SockAddress& peer);
    ~TcpConnection();

    EventLoop* loop() { return loop_; }
    const std::string& name() { return name_; }
    const SockAddress& local_addr() { return local_addr_; }
    const SockAddress& peer_addr() { return peer_addr_; }
    bool Connected() { return state_ == kConnected; }

    void Send(const void* data, int len);
    void Send(std::string_view data);
    void Send(Buffer* buf);

    void Close();
    void CloseInLoop();

    void setTcpNoDelay(bool on);
    void setConnectionCallback(const ConnectionCallback& cb) {
        connection_callback_ = cb;
    }
    void setMessageCallback(const MessageCallback& cb) {
        message_callback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
        write_complete_callback_ = cb;
    }
    void set_close_callback(const CloseCallback& cb) { close_callback_ = cb; }

    // called when TcpServer accepts a new connection, only once
    void ConnectionEstablished();
    // called when TcpServer removes TcpConnection from its map,
    // each TcpConnection will call once
    void ConnectionDestroyed();

   private:
    enum State {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting,
    };
    void HandleRead();
    void HandleWrite();
    // called in three case: 1. read return 0;
    // 2. call Close(); 3. epoll_wait return EPOLLHUP.
    void HandleClose();
    void HandleError();
    void SendInLoop(std::string_view data);
    void SendInLoop(const void* data, size_t len);
    const char* PrintState();

    EventLoop* loop_;
    std::string name_;
    State state_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const SockAddress local_addr_;
    const SockAddress peer_addr_;
    // callback when connection is connected and destroyed
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    CloseCallback close_callback_;
    Buffer input_buffer_;
    Buffer output_buffer_;
};

#endif  // MYWEBSERVER_TCP_CONNECTION_H