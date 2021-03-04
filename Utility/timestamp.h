//
// Created by rui836662398 on 2021/2/7.
//

#ifndef MYWEBSERVER_TIMESTAMP_H
#define MYWEBSERVER_TIMESTAMP_H

#include <sys/time.h>

#include <cstdint>
#include <cstdio>
#include <string>

class Timestamp {
   public:
    static const int kMicrosecondsPerSecond = 1000'000;

    Timestamp() : us_(0) {}
    explicit Timestamp(int64_t us) : us_(us) {}

    Timestamp operator+(double seconds) {
        int64_t diff = seconds * Timestamp::kMicrosecondsPerSecond;
        return Timestamp(us_ + diff);
    }
    Timestamp& operator+=(double seconds) {
        us_ += seconds * Timestamp::kMicrosecondsPerSecond;
        return *this;
    }
    Timestamp operator-(double seconds) {
        int64_t diff = seconds * Timestamp::kMicrosecondsPerSecond;
        return Timestamp(us_ - diff);
    }
    Timestamp& operator-=(double seconds) {
        us_ -= seconds * Timestamp::kMicrosecondsPerSecond;
        return *this;
    }
    double operator-(const Timestamp& rhs) {
        return (us_ - rhs.us_) * 1.0 / kMicrosecondsPerSecond;
    }
    bool operator<(const Timestamp& rhs) const { return us_ < rhs.us_; }
    bool operator>(const Timestamp& rhs) const { return us_ > rhs.us_; }
    bool operator<=(const Timestamp& rhs) const { return us_ <= rhs.us_; }
    bool operator>=(const Timestamp& rhs) const { return us_ >= rhs.us_; }
    bool operator==(const Timestamp& rhs) const { return us_ == rhs.us_; }

    std::string FormatS() const;
    std::string Format(bool show_us = true) const;

    int64_t us() const { return us_; }
    static Timestamp Now() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return Timestamp(tv.tv_sec * kMicrosecondsPerSecond + tv.tv_usec);
    }
    static Timestamp AddTime(Timestamp timestamp, double seconds) {
        int64_t diff = seconds * Timestamp::kMicrosecondsPerSecond;
        return Timestamp(timestamp.us() + diff);
    }

   private:
    // us_ gives the number of microseconds elapsed since the Epoch
    // 1970-01-01 00:00:00 +0000 (UTC).
    int64_t us_;
};


#endif  // MYWEBSERVER_TIMESTAMP_H
