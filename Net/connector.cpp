//
// Created by rui836662398 on 2021/3/4.
//

#include "connector.h"

#include "Net/Timer/timer.h"
#include "Net/channel.h"
#include "Net/event_loop.h"
#include "Net/socket.h"
#include "Utility/logging.h"

std::string unit_name = "Connector";

Connector::Connector(EventLoop* loop, const SockAddress& serverAddr)
    : loop_(loop),
      server_addr_(serverAddr),
      connect_(false),
      state_(kDisconnected),
      retry_delay_ms_(kInitRetryDelayMs) {
    TRACE("Ctor.");
}

Connector::~Connector() {
    TRACE("Dtor.");
    assert(!channel_);
}

void Connector::start() {
    connect_ = true;
    loop_->RunInLoop(
        std::bind(&Connector::StartInLoop, this));
}

void Connector::StartInLoop() {
    loop_->AssertInLoopThread();
    assert(state_ == kDisconnected);
    if (connect_) {
        Connect();
    } else {
        ERROR("StartInLoop() failed. Connector is off.");
    }
}

void Connector::stop() {
    connect_ = false;
    loop_->QueueInLoop(std::bind(&Connector::StopInLoop, this));
    if (auto timer = retry_timer_.lock()) {
        timer->set_deleted();
    }
}

void Connector::StopInLoop() {
    loop_->AssertInLoopThread();
    // effective only in kConnecting
    if (state_ == kConnecting) {
        setState(kDisconnected);
        int sockfd = RemoveAndResetChannel();
        Socket::close(sockfd);
    }
}

void Connector::Connect() {
    int sockfd = Socket::CreateSocketFd(server_addr_.family());
    int ret = ::connect(sockfd, server_addr_.get_sockaddr(),
                        static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno) {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            Connecting(sockfd);
            break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            Retry(sockfd);
            break;

        default:
            ERROR(fmt::format("connect() failed, error {}", savedErrno));
            Socket::close(sockfd);
            break;
    }
}

void Connector::restart() {
    loop_->AssertInLoopThread();
    setState(kDisconnected);
    retry_delay_ms_ = kInitRetryDelayMs;
    connect_ = true;
    StartInLoop();
}

void Connector::Connecting(int sockfd) {
    setState(kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->set_write_callback(std::bind(&Connector::HandleWrite, this));
    channel_->set_error_callback(std::bind(&Connector::HandleError, this));

    channel_->EnableWriting();
}

int Connector::RemoveAndResetChannel() {
    channel_->Remove();
    int sockfd = channel_->fd();
    // Can't reset channel_ here, because we are inside Channel::handleEvent
    loop_->QueueInLoop([this] { channel_.reset(); });
    return sockfd;
}

void Connector::HandleWrite() {
    TRACE(fmt::format("HandleWrite(), state {}", state_));

    if (state_ == kConnecting) {
        int sockfd = RemoveAndResetChannel();
        int err = Socket::getSocketError(sockfd);
        if (err) {
            ERROR(fmt::format("HandleWrite() - SO_ERROR = {} {}", err,
                              strerror_tl(err)));
            Retry(sockfd);
        } else if (Socket::IsSelfConnect(sockfd)) {
            ERROR("HandleWrite() - Self connect");
            Retry(sockfd);
        } else {
            setState(kConnected);
            if (connect_) {
                // connect succeed, transfer sockfd
                new_connection_callback_(sockfd);
            } else {
                Socket::close(sockfd);
            }
        }
    }
}

void Connector::HandleError() {
    ERROR(fmt::format("HandleError(), state {}", state_));
    if (state_ == kConnecting) {
        int sockfd = RemoveAndResetChannel();
        int err = Socket::getSocketError(sockfd);
        INFO(fmt::format("HandleError(), SO_ERROR {} {}", err,
                         strerror_tl(err)));
        Retry(sockfd);
    }
}

void Connector::Retry(int sockfd) {
    // reconnect must change sockfd
    Socket::close(sockfd);
    setState(kDisconnected);
    if (connect_) {
        INFO(fmt::format("Retry() - Retry connecting to {} in {} ms.",
                         server_addr_.IpPort(), retry_delay_ms_));
        retry_timer_ = loop_->RunAfter(
            retry_delay_ms_ / 1000.0,
            std::bind(&Connector::StartInLoop, shared_from_this()));
        // back-off
        retry_delay_ms_ = std::min(retry_delay_ms_ * 2, kMaxRetryDelayMs);
    } else {
        DEBUG("connector didn't start.");
    }
}
