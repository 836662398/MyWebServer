cmake_minimum_required(VERSION 3.17)
project(MyWebServer)

set(CMAKE_CXX_STANDARD 17)

find_package(spdlog REQUIRED)

include_directories(./)

add_executable(MyWebServer
        main.cpp
        Utility/noncopyable.h
        Utility/timestamp.h
        Utility/types.h
        Net/sock_addr.cpp Net/sock_addr.h
        Utility/logging.cpp Utility/logging.h Net/socket.cpp Net/socket.h)

target_link_libraries(
        MyWebServer
        spdlog::spdlog
)