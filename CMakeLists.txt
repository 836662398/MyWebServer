cmake_minimum_required(VERSION 3.10)
project(MyWebServer)

set(CMAKE_CXX_STANDARD 17)

#find_package(spdlog REQUIRED)
find_package(Threads REQUIRED)


include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(Utility)
add_subdirectory(Net)

add_executable(MyWebServer main.cpp)
target_link_libraries(MyWebServer lib_net)



