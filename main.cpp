#include <iostream>

#include "Net/Http/http_server.h"

#include "Utility/logging.h"

bool benchmark = true;

void OnRequest(const HttpRequest& req, HttpResponse* resp) {
    if (!benchmark) {
        std::cout << "Headers " << req.PrintMethod() << " " << req.path()
                  << std::endl;
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
    int num_thread = 4;
    // parse args
    int opt;
    const char *str = "t:b";
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch (opt) {
            case 't': {
                num_thread = atoi(optarg) - 1;
                std::cout<<"Thread's num is "<<num_thread + 1<<std::endl;
                break;
            }
            case 'b': {
                spdlog::get("log")->set_level(spdlog::level::off);
                std::cout<<"Benchmark starts."<<std::endl;
                break;
            }
            default:
                break;
        }
    }
    EventLoop loop;
    HttpServer server(&loop, 80, num_thread, "WebServer");
    server.set_response_callback(OnRequest);
    server.StartListening();
    std::cout << "Server starts" << std::endl;
    loop.Loop();
    return 0;
}