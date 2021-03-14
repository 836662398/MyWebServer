//
// Created by rui836662398 on 2021/2/24.
//

#ifndef MYWEBSERVER_EVENT_LOOP_H
#define MYWEBSERVER_EVENT_LOOP_H

#include <atomic>
#include <functional>
#include <thread>

#include "Net/Timer/timer_manager.h"
#include "Net/callbacks.h"
#include "Net/channel.h"
#include "Net/epoller.h"
#include "Utility/logging.h"

class EventLoop {
   public:
    using Callback = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void Loop();
    void Quit();

    // complete callback in this loop
    // thread-safe
    void RunInLoop(Callback cb);
    // queue callback in this loop
    // thread-safe
    void QueueInLoop(Callback cb);

    // wake up from epoll_wait
    void WakeUp();
    void UpdateChannel(Channel* channel, int operation);
    void RemoveChannel(Channel* channel);
    bool HasChannel(Channel* channel);

    // timers
    // thread safe

    // run callback at 'time'.
    TimerPtr RunAt(Timestamp time, TimerCallback cb);
    // run callback after delay seconds.
    TimerPtr RunAfter(double delay, TimerCallback cb);
    // run callback every interval seconds.
    TimerPtr RunEvery(double interval, TimerCallback cb);
    // cancel the timer.
    void Cancel(TimerPtr timer);

    bool IsInLoopThread();
    void AssertInLoopThread();
    static EventLoop* get_thread_local_eventloop();

   private:
    static const int kEpollTimeoutMs = 10000;

    void WakeUpReadHandle();
    void HandlePendingCallbacks();

    bool looping_;
    bool pending_callbacks_handling_;
    std::thread::id thread_id_;
    std::atomic<bool> quit_flag_;
    std::unique_ptr<Epoller> epoller_;
    std::vector<Channel*> active_channels_;
    TimerManager timer_manager_;

    int wakeup_fd_;
    std::unique_ptr<Channel> wakeup_channel_;
    std::mutex mutex_;
    std::vector<Callback> pending_callbacks_;
    int sequence_;

    static std::atomic<int> sequence_generator_;
};

#endif  // MYWEBSERVER_EVENT_LOOP_H
