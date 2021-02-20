cmake_minimum_required(VERSION 3.10)

SET(CMAKE_INSTALL_PREFIX ./build)

find_package(GTest REQUIRED)

include_directories(../)

add_executable(timestamp_test gtest_main.cc timestamp_test.cpp)
target_link_libraries(timestamp_test GTest::GTest GTest::Main)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)