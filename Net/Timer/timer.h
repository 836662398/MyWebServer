//
// Created by rui836662398 on 2021/3/3.
//

#ifndef MYWEBSERVER_TIMER_H
#define MYWEBSERVER_TIMER_H

#include <cassert>

#include "Net/callbacks.h"
#include "Utility/timestamp.h"

// interior class
class Timer {
   public:
    // interval is in seconds
    Timer(TimerCallback cb, Timestamp time, double interval = 0)
        : callback_(cb),
          deadline_(time),
          interval_(interval),
          repeat_(interval > 0),
          deleted_(false) {}

    void Run() const { callback_(); }
    Timestamp deadline() const { return deadline_; }
    bool repeat() const { return repeat_; }
    bool deleted() const { return deleted_; }
    void set_deleted() { deleted_ = true; }

    // return true if repeat
    bool Restart() {
        if (!repeat_ || deleted_) return false;
        deadline_ = deadline_ + interval_;
        return true;
    };

   private:
    const TimerCallback callback_;
    Timestamp deadline_;
    const double interval_;
    const bool repeat_;
    bool deleted_;
};

#endif  // MYWEBSERVER_TIMER_H
