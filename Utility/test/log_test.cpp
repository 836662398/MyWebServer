//
// Created by rui836662398 on 2021/2/26.
//

#include "Utility/logging.h"

#include <gtest/gtest.h>

static std::string unit_name = "test";

TEST(LogTest, test1){
    DEBUG("debug");
    INFO("info");
}

TEST(LogTest, test2){
    TRACE("trace");

}