//
// Created by rui836662398 on 2021/3/4.
//
#include "timestamp.h"

#include <string>

std::string Timestamp::FormatS() const {
    char buf[32] = {0};
    int64_t seconds = us_ / kMicrosecondsPerSecond;
    int64_t us = us_ % kMicrosecondsPerSecond;
    snprintf(buf, sizeof(buf), "%lld.%06lld s", seconds, us);
    return buf;
}

std::string Timestamp::Format(bool show_us) const {
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(us_ / kMicrosecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if (show_us) {
        int microseconds = static_cast<int>(us_ % kMicrosecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
    } else {
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}
