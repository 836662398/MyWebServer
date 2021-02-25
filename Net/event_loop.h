//
// Created by rui836662398 on 2021/2/24.
//

#ifndef MYWEBSERVER_EVENT_LOOP_H
#define MYWEBSERVER_EVENT_LOOP_H

#include <functional>

#include "Utility/logging.h"

class Channel;
class Epoller;

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
    void UpdateChannel(Channel *channel, int operation);
    void RemoveChannel(Channel *channel);
    bool HasChannel(Channel* channel);

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

    int wakeup_fd_;
    std::unique_ptr<Channel> wakeup_channel_;
    std::mutex mutex_;
    std::vector<Callback> pending_callbacks_;
};

#endif  // MYWEBSERVER_EVENT_LOOP_H
