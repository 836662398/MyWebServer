//
// Created by rui836662398 on 2021/2/26.
//
#include "Net/event_loop_thread_pool.h"

#include <gtest/gtest.h>

using namespace std;

TEST(EventLoopThreadPoolTest, test1){
    EventLoop base_loop;

    EventLoopThreadPool pool(&base_loop, 0);
    pool.Start();
    EXPECT_EQ(&base_loop, pool.GetNextLoop());
    EXPECT_EQ(&base_loop, pool.GetNextLoop());
    EXPECT_EQ(&base_loop, pool.GetNextLoop());
}

void print(){
    cout<<this_thread::get_id()<<endl;
}

TEST(EventLoopThreadPoolTest, test2){
    EventLoop base_loop;

    EventLoopThreadPool pool(&base_loop, 1);
    pool.Start();
    EventLoop* loop1 = pool.GetNextLoop();
    loop1->RunInLoop(&print);
    EXPECT_NE(loop1, &base_loop);
    EXPECT_EQ(loop1, pool.GetNextLoop());
    EXPECT_EQ(loop1, pool.GetNextLoop());
    EXPECT_EQ(loop1, pool.GetNextLoop());
    this_thread::sleep_for(1s);
}

TEST(EventLoopThreadPoolTest, test3){
    EventLoop base_loop;

    EventLoopThreadPool pool(&base_loop, 2);
    pool.Start();
    EventLoop* loop1 = pool.GetNextLoop();
    loop1->QueueInLoop(&print);
    EXPECT_NE(loop1, &base_loop);
    EXPECT_NE(loop1, pool.GetNextLoop());
    EXPECT_EQ(loop1, pool.GetNextLoop());
    EXPECT_NE(loop1, pool.GetNextLoop());
    EXPECT_EQ(loop1, pool.GetNextLoop());
    this_thread::sleep_for(1s);
}