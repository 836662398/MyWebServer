//
// Created by rui836662398 on 2021/3/1.
//

#ifndef MYWEBSERVER_ACCEPTOR_H
#define MYWEBSERVER_ACCEPTOR_H

#include <functional>

#include "Net/channel.h"
#include "Net/socket.h"
#include "Utility/noncopyable.h"

class EventLoop;
class SockAddress;

class Acceptor : noncopyable {
   public:
    using NewConnectionCallback =
        std::function<void(int sockfd, const SockAddress&)>;

    Acceptor(EventLoop* loop, const SockAddress& listenaddr, bool is_reuseport);
    ~Acceptor();

    void set_new_connection_callback(const NewConnectionCallback& cb) {
        new_connection_callback_ = cb;
    }

    void Listen();
    bool IsListening() { return listening_; }

   private:
    void HandleRead();

    EventLoop* loop_;
    Socket accept_socket_;
    Channel accept_channel_;
    NewConnectionCallback new_connection_callback_;
    bool listening_;
    // to close fd when fds overflow
    int idle_fd_;
};

#endif  // MYWEBSERVER_ACCEPTOR_H
