find_package(GTest REQUIRED)

set(GTEST_LIB GTest::GTest GTest::Main)

add_executable(sock_addr_test gtest_main.cc sock_addr_test.cpp)
target_link_libraries(sock_addr_test ${GTEST_LIB} lib_net)

add_executable(event_loop_test gtest_main.cc event_loop_test.cpp )
target_link_libraries(event_loop_test ${GTEST_LIB} lib_net)

add_executable(event_loop_thread_test gtest_main.cc event_loop_thread_test.cpp)
target_link_libraries(event_loop_thread_test ${GTEST_LIB} lib_net)

add_executable(event_loop_thread_pool_test gtest_main.cc event_loop_thread_pool_test.cpp)
target_link_libraries(event_loop_thread_pool_test ${GTEST_LIB} lib_net)

add_executable(buffer_test gtest_main.cc buffer_test.cpp)
target_link_libraries(buffer_test ${GTEST_LIB} lib_net)

add_executable(timer_manager_test gtest_main.cc timer_manager_test.cpp)
target_link_libraries(timer_manager_test ${GTEST_LIB} lib_net)