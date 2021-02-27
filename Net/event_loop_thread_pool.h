//
// Created by rui836662398 on 2021/2/26.
//

#ifndef MYWEBSERVER_EVENT_LOOP_THREAD_POOL_H
#define MYWEBSERVER_EVENT_LOOP_THREAD_POOL_H

#include "Net/event_loop_thread.h"
#include "Utility/noncopyable.h"

class EventLoopThreadPool : noncopyable {
   public:
    explicit EventLoopThreadPool(EventLoop* baseloop, int threadnum = 0,
                        const std::string& name = "");
    ~EventLoopThreadPool();

    void Start();

    EventLoop* GetNextLoop();

    const std::string& name() const { return name_; }

   private:
    EventLoop* base_loop_;
    std::string name_;
    int thread_num_;
    bool started_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

#endif  // MYWEBSERVER_EVENT_LOOP_THREAD_POOL_H
