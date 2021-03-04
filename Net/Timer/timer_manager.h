//
// Created by rui836662398 on 2021/3/3.
//

#ifndef MYWEBSERVER_TIMER_MANAGER_H
#define MYWEBSERVER_TIMER_MANAGER_H

#include <queue>

#include "Net/channel.h"
#include "Utility/noncopyable.h"
#include "timer.h"

class event_loop;

struct TimerCmp {
    bool operator()(const TimerPtr& a, const TimerPtr& b) const {
        return a->deadline() > b->deadline();
    }
};

class TimerManager : noncopyable {
   public:
    explicit TimerManager(EventLoop* loop);
    ~TimerManager();

    // repeats if interval > 0
    TimerPtr AddTimer(TimerCallback cb, Timestamp time, double interval = 0);
    void Cancel(TimerPtr timer);

   private:
    void AddTimerInLoop(TimerPtr timer);
    // call timerfd_settime() if necessary
    void Fresh();
    void ResetTimerfd(Timestamp deadline);
    // called when timerfd alarms
    void HandleRead();
    void ReadTimerfd();

    EventLoop* loop_;
    int timerfd_;
    // the earliest timestamp was set in timerfd_settime(),
    // no timestamp was set if earliest_time_set_ is Timestamp()
    Timestamp earliest_time_set_;
    bool no_time_set_;
    Channel timer_channel_;
    std::priority_queue<TimerPtr, std::vector<TimerPtr>, TimerCmp> timers_heap_;
};

#endif  // MYWEBSERVER_TIMER_MANAGER_H
