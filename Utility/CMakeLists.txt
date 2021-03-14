set(UTILITY_SRCS
    types.cpp
        logging.cpp timestamp.cpp)

#include_directories(./spdlog)

add_library(lib_utility ${UTILITY_SRCS})
target_link_libraries(lib_utility spdlog::spdlog pthread)

add_subdirectory(test)