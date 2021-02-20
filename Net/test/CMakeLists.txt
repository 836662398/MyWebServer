cmake_minimum_required(VERSION 3.10)

SET(CMAKE_INSTALL_PREFIX ./build)
set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)

include_directories(../../)

add_executable(sock_addr_test gtest_main.cc sock_addr_test.cpp ../sock_addr.cpp)
target_link_libraries(sock_addr_test GTest::GTest GTest::Main)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)