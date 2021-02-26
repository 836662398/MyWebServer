set(NET_SRCS
    sock_addr.cpp
    socket.cpp
    channel.cpp
    epoller.cpp
    event_loop.cpp
    event_loop_thread.cpp
)

add_library(lib_net ${NET_SRCS})
target_link_libraries(lib_net lib_utility)


add_subdirectory(test)