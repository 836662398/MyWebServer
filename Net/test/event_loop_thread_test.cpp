//
// Created by rui836662398 on 2021/2/26.
//
#include "Net/event_loop_thread.h"

#include <gtest/gtest.h>

using namespace std;

TEST(EventLoopThreadTest, test1) { EventLoopThread t; }

void print() { cout << "thread id: "<<std::this_thread::get_id() << endl; }

TEST(EventLoopThreadTest, test2) {
    EventLoopThread t;
    EventLoop* loop = t.StartLoop();
    EXPECT_NE(loop, nullptr);
    loop->QueueInLoop(&print);
    this_thread::sleep_for(1s);
}

TEST(EventLoopThreadTest, test3) {
    EventLoopThread t;
    EventLoop* loop = t.StartLoop();
    EXPECT_NE(loop, nullptr);
    loop->RunInLoop([loop] { loop->Quit(); });
    this_thread::sleep_for(1s);
}