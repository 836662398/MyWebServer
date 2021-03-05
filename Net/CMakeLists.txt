set(NET_SRCS
    sock_addr.cpp
    socket.cpp
    channel.cpp
    epoller.cpp
    event_loop.cpp
    event_loop_thread.cpp
    event_loop_thread_pool.cpp event_loop_thread_pool.h
    buffer.cpp buffer.h
    tcp_connection.cpp tcp_connection.h
    callbacks.h acceptor.cpp acceptor.h
    tcp_server.cpp tcp_server.h
    Timer/timer.cpp Timer/timer.h
    Timer/timer_manager.cpp Timer/timer_manager.h
    connector.cpp connector.h
    tcp_client.cpp tcp_client.h)

add_library(lib_net ${NET_SRCS})
target_link_libraries(lib_net lib_utility)


add_subdirectory(test)