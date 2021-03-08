//
// Created by ray on 2021/3/7.
//

#include "http_server.h"

#include "Utility/logging.h"

static std::string unit_name = "HttpServer";

HttpServer::HttpServer(EventLoop *loop, int16_t port, int thread_num,
                       const std::string &name, bool is_reuseport)
    : HttpServer(loop, SockAddress(port), thread_num, name, is_reuseport) {}

HttpServer::HttpServer(EventLoop *loop, const SockAddress &addr, int thread_num,
                       const std::string &name, bool is_reuseport)
    : server_(loop, addr, thread_num, name, is_reuseport),
      response_callback_(DefaultResponseCallback) {
    using namespace std::placeholders;
    server_.set_write_complete_callback(
        std::bind(&HttpServer::AfterWriting, this, _1));
    server_.set_connection_callback(
        std::bind(&HttpServer::OnConnection, this, _1));
    server_.set_message_callback(
        std::bind(&HttpServer::OnMessage, this, _1, _2));
}

void HttpServer::StartListening() {
    server_.StartListening();
    INFO(fmt::format("[{}] starts listening to {}", server_.name(),
                     server_.ip_port()));
}

void HttpServer::OnConnection(const TcpConnectionPtr &conn) {
    if (conn->Connected()) {
        conn2parser_[conn] = std::make_shared<HttpParser>();
    } else {
        int ret = conn2parser_.erase(conn);
        assert(ret == 1);
    }
}

void HttpServer::OnMessage(const TcpConnectionPtr &conn, Buffer *buf) {
    auto parser = conn2parser_[conn];
    assert(parser);

    if (!parser->ParseRequest(buf)) {
        conn->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
    }

    if (parser->ParseComplete()) {
        OnRequest(conn, parser->request());
    }
}

void HttpServer::OnRequest(const TcpConnectionPtr &conn,
                           const HttpRequest &req) {
    const std::string &connection = req.GetHeader("Connection");
    HttpResponse response(true);
    response_callback_(req, &response);
    Buffer buf;
    response.AppendToBuffer(&buf);
    conn->Send(&buf);
}

void HttpServer::AfterWriting(const TcpConnectionPtr &conn) {
    if (conn->Connected()) conn->Close();
}

void HttpServer::DefaultResponseCallback(const HttpRequest &,
                                         HttpResponse *resp) {
    resp->set_status_code(HttpResponse::k404NotFound);
    resp->set_status_message("Not Found");
    resp->set_short_connection();
}