//
// Created by rui836662398 on 2021/3/4.
//

#ifndef MYWEBSERVER_CONNECTOR_H
#define MYWEBSERVER_CONNECTOR_H

#include <functional>
#include <memory>

#include "Net/callbacks.h"
#include "Net/sock_addr.h"
#include "Utility/noncopyable.h"

class Channel;
class EventLoop;

class Connector : noncopyable, public std::enable_shared_from_this<Connector> {
   public:
    using NewConnectionCallback = std::function<void(int sockfd)>;

    Connector(EventLoop* loop, const SockAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        new_connection_callback_ = cb;
    }

    // thread safe
    void Start();
    // Stop retrying Connect() instead of Disconnect
    // thread safe
    void Stop();
    // must be called in loop thread
    void Restart();

    const SockAddress& server_address() const { return server_addr_; }

   private:
    enum States { kDisconnected, kConnecting, kConnected };
    static const int kMaxRetryDelayMs = 30 * 1000;
    static const int kInitRetryDelayMs = 500;

    void setState(States s) { state_ = s; }
    void StartInLoop();
    void StopInLoop();
    // call nonblocking Connect
    void Connect();
    // prepare to handle writable event
    void Connecting(int sockfd);
    void HandleWrite();
    void HandleError();
    void Retry(int sockfd);
    // remove channel and return fd for retrying or transfering fd
    int RemoveAndResetChannel();

    EventLoop* loop_;
    SockAddress server_addr_;
    bool connect_;
    States state_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback new_connection_callback_;
    int retry_delay_ms_;
    std::weak_ptr<Timer> retry_timer_;
};

#endif  // MYWEBSERVER_CONNECTOR_H
