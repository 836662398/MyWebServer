//
// Created by rui836662398 on 2021/2/26.
//

#include "event_loop_thread_pool.h"

#include "Utility/logging.h"

static std::string unit_name = "EventLoopThreadPool";

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop, int threadnum,
                                         const std::string &name)
    : base_loop_(baseloop),
      name_(name),
      thread_num_(threadnum),
      started_(false),
      next_(0) {
    if(thread_num_ < 0) FATAL_P("thread_num_ < 0 !");
}
EventLoopThreadPool::~EventLoopThreadPool() {
    TRACE("~EventLoopThreadPool().");
}

void EventLoopThreadPool::Start() {
    assert(!started_);
    base_loop_->AssertInLoopThread();
    started_ = true;
    for(int i=0;i<thread_num_;i++){
        threads_.push_back(std::make_unique<EventLoopThread>());
        loops_.push_back(threads_.back()->StartLoop());
    }
}

EventLoop *EventLoopThreadPool::GetNextLoop() {
    assert(started_);
    base_loop_->AssertInLoopThread();
    EventLoop* loop = base_loop_;
    if(!loops_.empty()){
        // round-robin
        loop = loops_[next_];
        next_ = (next_ + 1) % thread_num_;
    }
    return loop;
}