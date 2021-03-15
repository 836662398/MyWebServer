#include <signal.h>

#include <iostream>

#include "Net/Http/http_server.h"
#include "Utility/logging.h"

bool benchmark = false;

void OnRequest(const HttpRequest& req, HttpResponse* resp) {
//    if (!benchmark) {
//        std::cout << "Headers " << req.PrintMethod() << " " << req.path()
//                  << std::endl;
//        auto headers = req.headers();
//        for (const auto& header : headers) {
//            std::cout << header.first << ": " << header.second << std::endl;
//        }
//    }

    if (req.path() == "/") {
        resp->set_status_code(HttpResponse::k200Ok);
        resp->set_status_message("OK");
        resp->setContentType("text/html");
        resp->set_body(
            "<html><head><title>Xu Rui's Homepage</title></head>"
            "<body><h3>Xu Rui's Homepage</h3><p><a "
            "href=\"https://blog.csdn.net/qq_40586164\">Xu Rui on "
            "CSDN</a>.</body></html>");
    } else {
        resp->set_status_code(HttpResponse::k404NotFound);
        resp->set_status_message("Not Found");
        resp->set_short_connection();
    }
}

HttpServer* p_server;
void sig_handler(int sig) {
    if (sig) {
        printf("Server exit.");
        p_server->Stop();
        p_server = nullptr;
    }
}

int main(int argc, char** argv) {
    int num_thread = 4;
    // parse args
    int opt;
    const char* str = "t:b";
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch (opt) {
            case 't': {
                num_thread = atoi(optarg) - 1;
                std::cout << "Thread's num is " << num_thread + 1 << std::endl;
                break;
            }
            case 'b': {
                benchmark = true;
                spdlog::get("log")->set_level(spdlog::level::err);
                std::cout << "Benchmark starts." << std::endl;
                break;
            }
            default:
                break;
        }
    }
    EventLoop loop;
    HttpServer server(&loop, 8000, num_thread, "WebServer");
    server.set_response_callback(OnRequest);
    // HeartBeat
//    server.TurnOnHeartBeat(8);
    server.StartListening();

    p_server = &server;
    // Ctrl + C exit
    ::signal(SIGINT, sig_handler);
    std::cout << "Server starts" << std::endl;
    loop.Loop();
    return 0;
}