//
// Created by rui836662398 on 2021/3/1.
//

#include "acceptor.h"

#include <fcntl.h>

#include "Net/event_loop.h"
#include "Net/sock_addr.h"
#include "Utility/logging.h"

static std::string unit_name = "Acceptor";

Acceptor::Acceptor(EventLoop *loop, const SockAddress &listenaddr,
                   bool is_reuseport)
    : loop_(loop),
      accept_socket_(Socket::CreateSocketFd(listenaddr.family())),
      accept_channel_(loop_, accept_socket_.fd()),
      listening_(false),
      idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(idle_fd_ >= 0);
    accept_socket_.setReuseAddr(true);
    accept_socket_.setReusePort(is_reuseport);
    accept_socket_.Bind(listenaddr);
    accept_channel_.set_read_callback(std::bind(&Acceptor::HandleRead, this));
}
Acceptor::~Acceptor() {
    // accept_channel_.DisableAll(); //can be omitted
    accept_channel_.Remove();
}

void Acceptor::Listen() {
    loop_->AssertInLoopThread();
    listening_ = true;
    accept_socket_.Listen();
    accept_channel_.ETEnableReading();
}

void Acceptor::HandleRead() {
    loop_->AssertInLoopThread();
    SockAddress peer_addr;
    int connfd = 0;
    // accept in while is suitable for short connection
    while ((connfd = accept_socket_.accept(&peer_addr)) >= 0) {
        if (new_connection_callback_) {
            new_connection_callback_(connfd, peer_addr);
        } else {
            ERROR("new_connection_callback_ isn't set!");
            Socket::close(connfd);
        }
    }
    if(connfd < 0 && errno != EWOULDBLOCK){
        ERROR_P("accept() failed!");
        if (errno == EMFILE) {
            ::close(idle_fd_);
            idle_fd_ = ::accept(accept_socket_.fd(), NULL, NULL);
            ::close(idle_fd_);
            idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
