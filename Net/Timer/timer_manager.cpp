//
// Created by rui836662398 on 2021/3/3.
//

#include "timer_manager.h"

#include <sys/timerfd.h>

#include "Net/event_loop.h"
#include "Utility/logging.h"
#include "Utility/types.h"

static std::string unit_name = "TimerManager";

namespace {

int CreateTimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        FATAL("timerfd_create() failed!");
    }
    return timerfd;
}

struct timespec TimeFromNow(Timestamp time) {
    auto us = time.us() - Timestamp::Now().us();
    // to prevent timerfd_settime() failing due to negative time
    if (us < 100) us = 100;
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(us / Timestamp::kMicrosecondsPerSecond);
    ts.tv_nsec =
        static_cast<long>((us % Timestamp::kMicrosecondsPerSecond) * 1000);
    return ts;
}

}  // namespace

TimerManager::TimerManager(EventLoop *loop)
    : loop_(loop),
      timerfd_(CreateTimerfd()),
      timer_channel_(loop, timerfd_),
      earliest_time_set_(Timestamp()),
      no_timer_set_(false) {
    timer_channel_.set_read_callback(
        std::bind(&TimerManager::HandleRead, this));
    timer_channel_.EnableReading();
}

TimerManager::~TimerManager() {
    timer_channel_.Remove();
    ::close(timerfd_);
}

TimerPtr TimerManager::AddTimer(TimerCallback cb, Timestamp time,
                                double interval) {
    TimerPtr timer = std::make_shared<Timer>(cb, time, interval);
    loop_->RunInLoop(std::bind(&TimerManager::AddTimerInLoop, this, timer));
    return timer;
}

void TimerManager::Cancel(TimerPtr timer) {
    if (!timer->deleted()) timer->set_deleted();
}

void TimerManager::AddTimerInLoop(TimerPtr timer) {
    loop_->AssertInLoopThread();
    if (timer->deleted()) return;

    timers_heap_.push(timer);
    Fresh();
}

void TimerManager::Fresh() {
    if (timers_heap_.empty()) return;
    auto earliest_time = timers_heap_.top()->deadline();
    // there are three cases to timerfd_settime():
    // 1. first timer is added;
    // 2. earlier timer is added;
    // 3. timerfd was awaken up just now, new timer should be set
    if (earliest_time_set_ == Timestamp() ||
        earliest_time < earliest_time_set_ || no_timer_set_) {
        ResetTimerfd(earliest_time);
    }
}

void TimerManager::ResetTimerfd(Timestamp deadline) {
    struct itimerspec new_value;
    struct itimerspec old_value;
    memzero(&new_value, sizeof new_value);
    memzero(&old_value, sizeof old_value);
    new_value.it_value = TimeFromNow(deadline);
    int ret = ::timerfd_settime(timerfd_, 0, &new_value, &old_value);
    if (ret < 0)
        ERROR("timerfd_settime() failed!");
    else {
        earliest_time_set_ = deadline;
        no_timer_set_ = false;
    }
}

void TimerManager::HandleRead() {
    loop_->AssertInLoopThread();
    ReadTimerfd();

    while (!timers_heap_.empty()) {
        auto timer = timers_heap_.top();
        if (timer->deadline() <= Timestamp::Now()) {
            timers_heap_.pop();
            if (!timer->deleted()) {
                timer->Run();
                if (timer->Restart()) timers_heap_.push(timer);
            }
            no_timer_set_ = true;
        } else {
            break;
        }
    }
    Fresh();
}

void TimerManager::ReadTimerfd() {
    uint64_t howmany;
    ssize_t n = ::read(timerfd_, &howmany, sizeof howmany);
    if (n != sizeof howmany) {
        ERROR(fmt::format("timerfd read() return {} bytes instead of 8", n));
    }
}
