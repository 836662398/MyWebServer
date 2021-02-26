//
// Created by rui836662398 on 2021/2/25.
//

#ifndef MYWEBSERVER_EVENT_LOOP_THREAD_H
#define MYWEBSERVER_EVENT_LOOP_THREAD_H

#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>

#include "Net/event_loop.h"

#include "Utility/noncopyable.h"

// the thread for EventLoop to loop
class EventLoopThread : noncopyable{
   public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop* StartLoop();

   private:
    void LoopFunc();

    EventLoop* loop_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cv_;

};

#endif  // MYWEBSERVER_EVENT_LOOP_THREAD_H
