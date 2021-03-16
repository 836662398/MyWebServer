//
// Created by ray on 2021/3/7.
//

#include "http_server.h"

#include <any>

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
    server_.set_connection_callback(
        std::bind(&HttpServer::OnConnection, this, _1));
    server_.set_message_callback(
        std::bind(&HttpServer::OnMessage, this, _1, _2));
}

void HttpServer::TurnOnHeartBeat(double seconds) {
    server_.TurnOnHeartbeat(seconds);
    INFO(fmt::format("[{}] turns on heartbeat detection, timeout {} s",server_.name(), seconds));
}

void HttpServer::StartListening() {
    server_.StartListening();
    INFO(fmt::format("[{}] starts listening to {}", server_.name(),
                     server_.ip_port()));
}

void HttpServer::Stop() {
    INFO(fmt::format("[{}] Stop()", server_.name()));
    server_.Stop();
}

void HttpServer::OnConnection(const TcpConnectionPtr &conn) {
    if (conn->Connected()) conn->set_something(HttpParser());
}

void HttpServer::OnMessage(const TcpConnectionPtr &conn, Buffer *buf) {
    auto parser = std::any_cast<HttpParser>(conn->p_something());
    if (!parser->ParseRequest(buf)) {
        conn->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->Close();
    }

    if (parser->ParseComplete()) {
        OnRequest(conn, parser->request());
        parser->Reset();
    }
}

void HttpServer::OnRequest(const TcpConnectionPtr &conn,
                           const HttpRequest &req) {
    const std::string &connection = req.GetHeader("Connection");
    bool close =
        connection == "close" ||
        (req.version() == HttpRequest::kHttp10 && connection != "keep-alive");
    bool is_short = (close);
    HttpResponse response(is_short);
    response_callback_(req, &response);
    Buffer buf;
    // sent message generation by response
    response.AppendToBuffer(&buf);
    conn->Send(&buf);
    if (response.IsShortConnection()) {
        conn->Shutdown();
    }
}


void HttpServer::DefaultResponseCallback(const HttpRequest &,
                                         HttpResponse *resp) {
    resp->set_status_code(HttpResponse::k404NotFound);
    resp->set_status_message("Not Found");
    resp->set_short_connection();
}

