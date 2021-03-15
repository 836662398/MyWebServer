find_package(GTest)
if(GTest_FOUND)
add_executable(timestamp_test gtest_main.cc timestamp_test.cpp)
target_link_libraries(timestamp_test GTest::GTest GTest::Main lib_utility)

add_executable(log_test gtest_main.cc log_test.cpp log_test.cpp)
target_link_libraries(log_test GTest::GTest GTest::Main lib_utility)
endif()