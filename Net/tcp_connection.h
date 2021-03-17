//
// Created by rui836662398 on 2021/3/1.
//

#ifndef MYWEBSERVER_TCP_CONNECTION_H
#define MYWEBSERVER_TCP_CONNECTION_H

#include <any>
#include <memory>

#include "Net/Timer/timer.h"
#include "Net/buffer.h"
#include "Net/callbacks.h"
#include "Net/channel.h"
#include "Net/sock_addr.h"
#include "Net/socket.h"
#include "Utility/noncopyable.h"
#include "Utility/timestamp.h"

class Channel;
class EventLoop;
class Socket;

// wrapper of established connections
// Invisible to users
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
   public:
    TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                  const SockAddress& local, const SockAddress& peer,
                  double heartbeat_timeout = 0);
    ~TcpConnection();

    void Init();
    static TcpConnectionPtr CreateTcpConnPtr(EventLoop* loop, const std::string& name,
                                      int sockfd, const SockAddress& local,
                                      const SockAddress& peer,
                                      double heartbeat_timeout = 0);

    EventLoop* loop() { return loop_; }
    const std::string& name() { return name_; }
    const SockAddress& local_addr() { return local_addr_; }
    const SockAddress& peer_addr() { return peer_addr_; }
    bool Connected() { return state_ == kConnected; }

    void Send(const void* data, int len);
    void Send(std::string_view data);
    void Send(Buffer* buf);

    // close util writing is finished
    void Close();
    void ForceClose();
    void Shutdown();

    void setTcpNoDelay(bool on);
    void set_connection_callback(const ConnectionCallback& cb) {
        connection_callback_ = cb;
    }
    void set_message_callback(const MessageCallback& cb) {
        message_callback_ = cb;
    }
    void set_write_complete_callback(const WriteCompleteCallback& cb) {
        write_complete_callback_ = cb;
    }
    // be called by TcpServer or TcpClient
    // their close_callback_ erase their TcpConnection members
    // and call ConnDestroy() to remove channel
    void set_close_callback(const CloseCallback& cb) { close_callback_ = cb; }

    // called when TcpServer or TcpClient accepts/Connect a new connection,
    // only once
    void ConnEstablished();
    // called when TcpServer or TcpClient removes TcpConnection from its map,
    // each TcpConnection will call once
    void ConnDestroy();

    void set_something(const std::any& something) { something_ = something; }

    const std::any& something() const { return something_; }

    std::any* p_something() { return &something_; }

    static void DefaultConnCallback(const TcpConnectionPtr& conn);
    static void DefaultMessageCallback(const TcpConnectionPtr& conn,
                                       Buffer* buffer);

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
    // 2. call ForceClose(); 3. epoll_wait return EPOLLHUP.
    void HandleClose();
    void HandleError();
    void SendInLoop(std::string_view data);
    void SendInLoop(const void* data, size_t len);
    void CloseInLoop();
    void ForceCloseInLoop();
    void ShutdownInLoop();
    const char* PrintState();

    EventLoop* loop_;
    std::string name_;
    State state_;
    Socket socket_;
    Channel channel_;
    const SockAddress local_addr_;
    const SockAddress peer_addr_;
    // callback when connection is connected and destroyed
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    CloseCallback close_callback_;
    Buffer input_buffer_;
    Buffer output_buffer_;
    // to judge Close() or Shutdown()
    bool is_closed_;
    std::shared_ptr<Timer> heartbeat_timer_;
    double heartbeat_timeout_s_;
    std::function<void()> heartbeat_callback_;
    // something could be used
    std::any something_;
};

#endif  // MYWEBSERVER_TCP_CONNECTION_H
