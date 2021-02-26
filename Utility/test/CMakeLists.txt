find_package(GTest REQUIRED)

add_executable(timestamp_test gtest_main.cc timestamp_test.cpp)
target_link_libraries(timestamp_test GTest::GTest GTest::Main lib_utility)

