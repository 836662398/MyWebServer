#include <iostream>

#include "Net/Http/http_server.h"

bool benchmark = false;

void OnRequest(const HttpRequest& req, HttpResponse* resp) {
    std::cout << "Headers " << req.PrintMethod() << " " << req.path()
              << std::endl;
    if (!benchmark) {
        auto headers = req.headers();
        for (const auto& header : headers) {
            std::cout << header.first << ": " << header.second << std::endl;
        }
    }

    if (req.path() == "/") {
        resp->set_status_code(HttpResponse::k200Ok);
        resp->set_status_message("OK");
        resp->setContentType("text/html");
        resp->set_body(
            "<html><head><title>Xu Rui's Homepage</title></head>"
            "<body><h4>Xu Rui's Homepage</h4><p><a "
            "href=\"https://blog.csdn.net/qq_40586164\">Xu Rui on "
            "CSDN</a>.</body></html>");
    } else {
        resp->set_status_code(HttpResponse::k404NotFound);
        resp->set_status_message("Not Found");
        resp->set_short_connection();
    }
}

int main(int argc, char** argv) {
    int numThreads = 11;
    if (argc > 1) {
        benchmark = true;
        numThreads = atoi(argv[1]);
    }
    EventLoop loop;
    HttpServer server(&loop, 80, numThreads, "WebServer");
    server.set_response_callback(OnRequest);
    server.StartListening();
    std::cout<<"Server starts"<<std::endl;
    loop.Loop();
    return 0;
}