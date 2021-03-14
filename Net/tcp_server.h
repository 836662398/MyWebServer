//
// Created by rui836662398 on 2021/3/2.
//

#ifndef MYWEBSERVER_TCP_SERVER_H
#define MYWEBSERVER_TCP_SERVER_H

#include <atomic>
#include <unordered_map>

#include "Net/acceptor.h"
#include "Net/callbacks.h"
#include "Net/event_loop_thread_pool.h"
#include "Net/sock_addr.h"
#include "Net/tcp_connection.h"
#include "Utility/noncopyable.h"

class TcpServer : noncopyable {
   public:
    // thread_num: num of threads in EventLoopThreadPool
    // - 0 means all I/O in loop's thread, no thread will created.
    //   this is the default value.
    // - 1 means all I/O in another thread.
    // - N means a thread pool with N threads, new connections
    //   are assigned on a round-robin basis.
    TcpServer(EventLoop* loop, uint16_t port, int thread_num = 0,
              const std::string& name = "", bool is_reuseport = false);
    TcpServer(EventLoop* loop, const SockAddress& listen_addr,
              int thread_num = 0, const std::string& name = "",
              bool is_reuseport = false);
    ~TcpServer();

    const std::string& ip_port() const { return ip_port_; }
    const std::string& name() const { return name_; }
    EventLoop* loop() const { return loop_; }

    void set_connection_callback(const ConnectionCallback& cb) {
        connection_callback_ = cb;
    }
    void set_message_callback(const MessageCallback& cb) {
        message_callback_ = cb;
    }
    void set_write_complete_callback(const WriteCompleteCallback& cb) {
        write_complete_callback_ = cb;
    }

    void TurnOnHeartbeat(double seconds) { heartbeat_timeout_s_ = seconds; }
    double heartbeat_timeout_s() { return heartbeat_timeout_s_; }

    void StartListening();

   private:
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    void HandleNewConn(int sockfd, const SockAddress& peer_addr);
    void RemoveConn(const TcpConnectionPtr& conn);
    void RemoveConnInLoop(const TcpConnectionPtr& conn);

    EventLoop* loop_;  // the loop of acceptor
    const SockAddress listen_addr_;
    const std::string ip_port_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;  // avoid revealing Acceptor
    std::unique_ptr<EventLoopThreadPool> thread_pool_;
    // three callbacks for new connections
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    double heartbeat_timeout_s_ = 0;

    std::atomic<bool> started_;
    int next_conn_id_;
    ConnectionMap connections_;
};

#endif  // MYWEBSERVER_TCP_SERVER_H
