//
// Created by ray on 2021/3/7.
//

#ifndef MYWEBSERVER_HTTP_SERVER_H
#define MYWEBSERVER_HTTP_SERVER_H

#include <map>
#include <unordered_map>

#include "Net/Http/http_parser.h"
#include "Net/Http/http_request.h"
#include "Net/Http/http_response.h"
#include "Net/tcp_server.h"

class HttpServer : noncopyable {
   public:
    using HttpParserPtr = std::shared_ptr<HttpParser>;
    using ResponseCallback =
        std::function<void(const HttpRequest&, HttpResponse*)>;

    HttpServer(EventLoop* loop, int16_t port, int thread_num,
               const std::string& name, bool is_reuseport = false);
    HttpServer(EventLoop* loop, const SockAddress& addr, int thread_num,
               const std::string& name, bool is_reuseport = false);

    void set_response_callback(const ResponseCallback& cb) {
        response_callback_ = cb;
    }

    void TurnOnHeartBeat(double seconds);
    void StartListening();

    static void DefaultResponseCallback(const HttpRequest&, HttpResponse* resp);

   private:
    void OnConnection(const TcpConnectionPtr& conn);
    void OnMessage(const TcpConnectionPtr& conn, Buffer* buf);
    void OnRequest(const TcpConnectionPtr& conn, const HttpRequest& req);

    TcpServer server_;
    // Response generation by request
    ResponseCallback response_callback_;
};

#endif  // MYWEBSERVER_HTTP_SERVER_H
