set(UTILITY_SRCS
    logging.cpp
    types.cpp
        )

add_library(lib_utility ${UTILITY_SRCS})
target_link_libraries(lib_utility spdlog::spdlog pthread)

add_subdirectory(test)