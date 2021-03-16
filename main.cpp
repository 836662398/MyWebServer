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
    //            std::cout << header.first << ": " << header.second <<
    //            std::endl;
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
        printf("Server exit.\n");
        p_server->Stop();
        p_server = nullptr;
    }
}

int main(int argc, char** argv) {
    int process_num = 2;
    int sub_thread_num = 4;
    int heartbeat_s = 0;  // parse args
    int opt;
    const char* str = "p:s:h:b";
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch (opt) {
            case 'p': {
                process_num = atoi(optarg);
                break;
            }
            case 's': {
                sub_thread_num = atoi(optarg);
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
    std::cout << "Process num is " << process_num << std::endl;
    std::cout << "Sub thread's num is " << sub_thread_num << std::endl;
    pid_t pid;
    std::atomic<int> sequence_ = 0;
    std::string name = "Server0";
    printf("WebServer 0 starts\n");
    for (int i = 1; i < process_num; i++) {
        ++sequence_;
        pid = fork();
        if (pid == 0) {
            printf("WebServer %d starts\n", sequence_.load());
            fflush(stdout);
            name = "Server" + std::to_string(sequence_);
            break;
        }
        if (pid < 0) printf("fork() failed.");
    }
    EventLoop loop;

    HttpServer server(&loop, 8000, sub_thread_num, name, true);
    server.set_response_callback(OnRequest);
    // HeartBeat
    if (heartbeat_s > 0) server.TurnOnHeartBeat(heartbeat_s);
    server.StartListening();

    p_server = &server;
    // Ctrl + C exit
    ::signal(SIGINT, sig_handler);

    loop.Loop();
    return 0;
}