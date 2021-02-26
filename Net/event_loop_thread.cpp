//
// Created by rui836662398 on 2021/2/25.
//

#include "event_loop_thread.h"

#include "Net/event_loop.h"

EventLoopThread::EventLoopThread() : loop_(nullptr) {}

EventLoopThread::~EventLoopThread() {
    if (loop_ != nullptr) loop_->Quit();
    if (thread_.joinable()) thread_.join();
}

EventLoop* EventLoopThread::StartLoop() {
    std::thread t(std::bind(&EventLoopThread::LoopFunc, this));
    thread_ = std::move(t);

    {
        std::unique_lock<std::mutex> lck(mutex_);
        cv_.wait(lck, [this] { return loop_ != nullptr; });
    }

    return loop_;
}

void EventLoopThread::LoopFunc() {
    EventLoop loop;
    {
        std::unique_lock<std::mutex> lck(mutex_);
        loop_ = &loop;
        cv_.notify_one();
    }
    loop.Loop();
    std::unique_lock<std::mutex> lck(mutex_);
    loop_ = nullptr;
}
