//
// Created by rui836662398 on 2021/2/25.
//
#include "Net/event_loop.h"

#include <gtest/gtest.h>

using namespace std;

void fun(EventLoop* loop1) {
    this_thread::sleep_for(1s);
    loop1->Quit();
    assert(EventLoop::get_thread_local_eventloop() == nullptr);
    EventLoop loop;
    assert(EventLoop::get_thread_local_eventloop() == &loop);
}

TEST(EventLoopTest, test1) {
    EXPECT_TRUE(EventLoop::get_thread_local_eventloop() == nullptr);
    EventLoop loop;
    EXPECT_EQ(&loop, EventLoop::get_thread_local_eventloop());
    EXPECT_TRUE(loop.IsInLoopThread());
    loop.AssertInLoopThread();
    std::thread t(&fun, &loop);
    loop.Loop();
    t.join();
}

void fun2(EventLoop* loop1) {
    this_thread::sleep_for(1s);
    loop1->RunInLoop(bind(&EventLoop::Quit, loop1));
}

TEST(EventLoopTest, test2) {
    EventLoop loop;
    loop.AssertInLoopThread();
    std::thread t(&fun2, &loop);
    loop.Loop();
    t.join();
}

TEST(EventLoopTest, test3) {
    EventLoop loop;
    loop.QueueInLoop([&loop] { loop.Quit(); });
    loop.Loop();
}