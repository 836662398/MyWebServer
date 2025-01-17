//
// Created by rui836662398 on 2021/3/1.
//

#include "tcp_connection.h"

#include "Net/channel.h"
#include "Net/event_loop.h"
#include "Net/socket.h"
#include "Utility/logging.h"

static std::string unit_name = "TcpConnection";

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name,
                             int sockfd, const SockAddress &local,
                             const SockAddress &peer,
                             double heartbeat_timeout_s)
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      socket_(sockfd),
      channel_(loop, sockfd),
      local_addr_(local),
      peer_addr_(peer),
      is_closed_(false),
      heartbeat_timeout_s_(heartbeat_timeout_s) {
    assert(loop != nullptr);
    channel_.set_read_callback(std::bind(&TcpConnection::HandleRead, this));
    channel_.set_write_callback(std::bind(&TcpConnection::HandleWrite, this));
    channel_.set_close_callback(std::bind(&TcpConnection::HandleClose, this));
    channel_.set_error_callback(std::bind(&TcpConnection::HandleError, this));

    TRACE(fmt::format("ctor [{}], fd = {}, loop sequence {}", name_, sockfd,
                      loop_->sequence()));
}

TcpConnection::~TcpConnection() {
    TRACE(fmt::format("dtor [{}], fd = {}, state = {}", name_, channel_.fd(),
                      PrintState()));
    assert(state_ == kDisconnected);
}

// weak_from_this() whithin ctor will be empty,
// so we should use Init() after ctor.
// https://stackoverflow.com/questions/50557861/weak-from-this-within-constructor
void TcpConnection::Init() {
    heartbeat_callback_ = [ptr = weak_from_this()] {
        if (std::shared_ptr<TcpConnection> sp = ptr.lock()) {
            DEBUG("timer timeout");
            sp->Close();
        }
    };
    if (heartbeat_timeout_s_ > 0) {
        heartbeat_timer_ =
            loop_->RunAfter(heartbeat_timeout_s_, heartbeat_callback_);
    }
}

TcpConnectionPtr TcpConnection::CreateTcpConnPtr(EventLoop *loop,
                                                 const std::string &name,
                                                 int sockfd,
                                                 const SockAddress &local,
                                                 const SockAddress &peer,
                                                 double heartbeat_timeout) {
    auto sp = std::make_shared<TcpConnection>(loop, name, sockfd, local, peer,
                                              heartbeat_timeout);
    sp->Init();
    return sp;
}

void TcpConnection::Send(const void *data, int len) {
    Send(std::string_view(static_cast<const char *>(data), len));
}
void TcpConnection::Send(std::string_view data) {
    if (state_ != kConnected) {
        ERROR(fmt::format("[{}] isn't connected, fail to Send()", name_));
        return;
    }
    if (loop_->IsInLoopThread()) {
        SendInLoop(data);
    } else {
        // data's lifespan cannot be guaranteed, should be stored
        loop_->RunInLoop([this, str = std::string(data)] {
            SendInLoop(str);
        });  // should be message
        // std::forward<string>(message)));
    }
}
void TcpConnection::Send(Buffer *buf) {
    if (state_ != kConnected) {
        ERROR(fmt::format("[{}] isn't connected, fail to Send()", name_));
        return;
    }
    if (loop_->IsInLoopThread()) {
        SendInLoop(buf->ReadData(), buf->ReadableBytes());
    } else {
        // buf's lifespan cannot be guaranteed, should be stored
        loop_->RunInLoop([this, str = buf->TakeOutAll()] { SendInLoop(str); });
    }
}

void TcpConnection::SendInLoop(std::string_view message) {
    SendInLoop(message.data(), message.size());
}
void TcpConnection::SendInLoop(const void *data, size_t len) {
    loop_->AssertInLoopThread();
    ssize_t nwritten = 0;
    size_t remaining = len;
    bool faultError = false;
    if (state_ != kConnected) {
        ERROR(fmt::format("[{}] isn't connected, fail to Send()", name_));
        return;
    }
    // if nothing is in output queue, try writing directly
    if (!channel_.IsWriting() && output_buffer_.ReadableBytes() == 0) {
        while ((nwritten = ::write(channel_.fd(), data, len)) > 0) {
            remaining = len - nwritten;
            if (remaining == 0) {
                if (write_complete_callback_)
                    loop_->QueueInLoop(std::bind(write_complete_callback_,
                                                 shared_from_this()));
                break;
            }
        }
        if (nwritten < 0) {
            nwritten = 0;
            if (errno != EWOULDBLOCK) {
                ERROR_P("write()");
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    assert(remaining <= len);
    if (!faultError && remaining > 0) {
        size_t oldLen = output_buffer_.ReadableBytes();
        output_buffer_.Append(static_cast<const char *>(data) + nwritten,
                              remaining);
        if (!channel_.IsWriting()) {
            channel_.EnableWriting();
        }
    }
}

void TcpConnection::Close() {
    if (state_ == kConnected) {
        state_ = kDisconnecting;
        is_closed_ = true;
        TRACE(fmt::format("[{}] Close()", name_));
        loop_->RunInLoop(
            std::bind(&TcpConnection::CloseInLoop, shared_from_this()));
    }
}

void TcpConnection::CloseInLoop() {
    loop_->AssertInLoopThread();
    if (!channel_.IsWriting()) {
        if (state_ == kDisconnecting) HandleClose();
    }
    // otherwise close when write is completed.
}

void TcpConnection::ForceClose() {
    if (state_ == kConnected || state_ == kDisconnecting) {
        loop_->QueueInLoop(
            std::bind(&TcpConnection::ForceCloseInLoop, shared_from_this()));
    }
}
void TcpConnection::ForceCloseInLoop() {
    loop_->AssertInLoopThread();
    if (state_ == kConnected || state_ == kDisconnecting) {
        state_ = kDisconnecting;
        DEBUG(fmt::format("[{}] ForceClose()", name_));
        HandleClose();
    }
}

void TcpConnection::Shutdown() {
    if (state_ == kConnected) {
        state_ = kDisconnecting;
        loop_->RunInLoop(std::bind(&TcpConnection::ShutdownInLoop, this));
    }
}
void TcpConnection::ShutdownInLoop() {
    loop_->AssertInLoopThread();
    if (!channel_.IsWriting()) {
        // we are not writing
        socket_.ShutdownWrite();
    }
}

void TcpConnection::setTcpNoDelay(bool on) { socket_.setTcpNoDelay(on); }

void TcpConnection::ConnEstablished() {
    loop_->AssertInLoopThread();
    assert(state_ == kConnecting);
    state_ = kConnected;
    socket_.setTcpNoDelay(true);
    channel_.ETEnableReading();
    connection_callback_(shared_from_this());
    TRACE("ConnEstablished()");
}

void TcpConnection::ConnDestroy() {
    loop_->AssertInLoopThread();
    if (heartbeat_timer_) {
        loop_->Cancel(heartbeat_timer_);
        heartbeat_timer_ = nullptr;
    }
    if (state_ == kConnected) {
        state_ = kDisconnected;
        channel_.DisableAll();  // can be omitted
        connection_callback_(shared_from_this());
    }
    INFO(fmt::format("[{}] disconnects.", name_));
    channel_.Remove();
}

void TcpConnection::HandleRead() {
    loop_->AssertInLoopThread();
    int saved_errno = 0;
    ssize_t n;
    // It's likely that ReadFd return 0 after message_callback has been called
    // HandleClose() will be called twice !
    while ((n = input_buffer_.ReadFd(channel_.fd(), &saved_errno)) > 0)
        message_callback_(shared_from_this(), &input_buffer_);
    if (n == 0) {
        INFO(fmt::format("[{}] read return 0.", name_));
        HandleClose();
        return;
    } else {
        if (saved_errno != EWOULDBLOCK) {
            errno = saved_errno;
            ERROR_P("ReadFd() failed!");
            HandleError();
        }
    }
    if (heartbeat_timer_) {
        loop_->Cancel(heartbeat_timer_);
        heartbeat_timer_ =
            loop_->RunAfter(heartbeat_timeout_s_, heartbeat_callback_);
    }
}

void TcpConnection::HandleWrite() {
    loop_->AssertInLoopThread();
    if (output_buffer_.ReadableBytes() == 0) {
        DEBUG("No need to write, but was awaken by epoll wait.");
        return;
    }
    if (channel_.IsWriting()) {
        ssize_t n;
        while ((n = ::write(channel_.fd(), output_buffer_.ReadData(),
                            output_buffer_.ReadableBytes())) > 0) {
            output_buffer_.Skip(n);
            if (output_buffer_.ReadableBytes() == 0) {
                channel_.DisableWriting();
                if (write_complete_callback_) {
                    loop_->QueueInLoop(std::bind(write_complete_callback_,
                                                 shared_from_this()));
                }
                // ShutdownInLoop() has been called
                if (state_ == kDisconnecting) {
                    if(is_closed_)
                        CloseInLoop();
                    else
                        ShutdownInLoop();
                }
                break;
            }
        }
        if (n <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            ERROR_P("write() failed!");
        }
    } else {
        ERROR(fmt::format("[{}] can't write!", name_));
    }
}

void TcpConnection::HandleClose() {
    loop_->AssertInLoopThread();
    if (heartbeat_timer_) {
        loop_->Cancel(heartbeat_timer_);
        heartbeat_timer_ = nullptr;
    }
    if (state_ == kDisconnected) return;
    TRACE(fmt::format("[{}] HandleClose().", name_));
    assert(state_ != kConnecting);
    // we don't close fd, leave it to Socket dtor
    state_ = kDisconnected;
    channel_.DisableAll();

    TcpConnectionPtr guard(shared_from_this());
    connection_callback_(guard);
    // close_callback_() couble be
    // TcpServer::RemoveConn() or
    // TcpClient::RemoveConn()
    close_callback_(guard);
}

void TcpConnection::HandleError() {
    int err = Socket::getSocketError(channel_.fd());
    ERROR(
        fmt::format("[{}] HandleError(). error - {}", name_, strerror_tl(err)));
}

const char *TcpConnection::PrintState() {
    switch (state_) {
        case kDisconnected:
            return "kDisconnected";
        case kConnecting:
            return "kConnecting";
        case kConnected:
            return "kConnected";
        case kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
    }
}

void TcpConnection::DefaultConnCallback(const TcpConnectionPtr &conn) {
    TRACE(fmt::format("{} -> {} is {}.", conn->local_addr().IpPort(),
                      conn->peer_addr().IpPort(),
                      conn->Connected() ? "UP" : "DOWN"));
}

void TcpConnection::DefaultMessageCallback(const TcpConnectionPtr &conn,
                                           Buffer *buffer) {
    buffer->Reset();
    INFO("No MessageCallback was set!");
}
