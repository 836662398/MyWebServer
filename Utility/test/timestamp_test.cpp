//
// Created by rui836662398 on 2021/2/8.
//
#include "Utility/timestamp.h"

#include <gtest/gtest.h>

#include <chrono>

TEST(TimestampTest, test) {
    auto t = Timestamp();
    auto now = Timestamp::Now();
    EXPECT_TRUE(t < now);

    int64_t c_s = time(nullptr);
    int64_t c11_us = std::chrono::duration_cast<std::chrono::microseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();
    int64_t ts_us = now.us();
    EXPECT_TRUE(c_s == ts_us / Timestamp::kMicrosecondsPerSecond);
    EXPECT_TRUE(c11_us / 1000 == ts_us / 1000);

    auto after = now + 2;
    double interval = after - now;
    EXPECT_TRUE(interval == 2);

    auto before = now - 2;
    double interval1 = before - now;
    EXPECT_TRUE(interval1 == -2);
}

TEST(TimestampTest, test1) {
    const int kNumber = 1000 * 1000;

    std::vector<Timestamp> stamps;
    stamps.reserve(kNumber);
    for (int i = 0; i < kNumber; ++i) {
        stamps.push_back(Timestamp::Now());
    }
    for (int i = 1; i < kNumber; ++i) {
        EXPECT_TRUE(stamps[i] >= stamps[i - 1]);
        double inc = stamps[i] - stamps[i - 1];
        if (inc > 0.001) {
            printf("big gap %lf\n", inc);
        }
    }
}