//
// Created by rui836662398 on 2021/2/9.
//

#ifndef MYWEBSERVER_LOGGING_H
#define MYWEBSERVER_LOGGING_H

#include <spdlog/spdlog.h>

#define TRACE(x) spdlog::get("log")->trace("[{}]{}", unit_name, x)
#define DEBUG(x) spdlog::get("log")->debug("[{}]{}", unit_name, x)
#define INFO(x) spdlog::get("log")->info("[{}]{}", unit_name, x)
#define ERROR(x) spdlog::get("log")->error("[{}]{}", unit_name, x)
#define FATAL(x) spdlog::get("log")->error("[{}]{} System exit.", unit_name, x);abort()



#endif  // MYWEBSERVER_LOGGING_H
