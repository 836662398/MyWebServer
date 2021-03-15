set(UTILITY_SRCS
        types.cpp
        logging.cpp timestamp.cpp)



add_library(lib_utility ${UTILITY_SRCS})

find_package(spdlog)

if (spdlog_FOUND)
    target_link_libraries(lib_utility spdlog::spdlog pthread)
else ()
    target_link_libraries(lib_utility pthread)
    message("spdlog lib was not found. Header-only lib will be used.Please install spdlog static lib if you want to Accelerate compilation.")
endif ()


add_subdirectory(test)