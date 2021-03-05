//
// Created by rui836662398 on 2021/3/4.
//
#include "Net/Timer/timer_manager.h"

#include <gtest/gtest.h>

#include "Net/event_loop.h"
#include "Net/event_loop_thread.h"

using namespace std;

EventLoop* g_loop;
int x = 1;

void Func1() {
    EXPECT_EQ(x, 1);
    printf("Func1 %s\n", Timestamp::Now().FormatS().c_str());
}
void Func2() {
    x = 2;
    printf("Func2 %s\n", Timestamp::Now().FormatS().c_str());
}
void Func3() {
    EXPECT_EQ(x, 2);
    printf("Func3 %s\n", Timestamp::Now().FormatS().c_str());
}
void Quit() {
    g_loop->Quit();
    printf("Quit  %s\n", Timestamp::Now().FormatS().c_str());
}

TEST(TimerTest, test1) {
    EventLoop loop;
    g_loop = &loop;
    loop.RunAfter(1.0, Func1);
    loop.RunAfter(1.1, Func2);
    loop.RunAfter(1.2, Func3);
    auto timer = loop.RunAfter(1.3, Func2);
    loop.Cancel(timer);
    loop.RunAfter(1.3, Func3);
    loop.RunAt(Timestamp::Now() + 1.4, Quit);
    loop.RunAfter(1.5, Func1);
    loop.Loop();
}

void Print() {
    ++x;
    printf("Time: %s\n", Timestamp::Now().FormatS().c_str());
}

void Jundge(int num) { EXPECT_EQ(x, num); }

TEST(TimerTest, test2) {
    x = 0;
    EventLoopThread loopThread;
    EventLoop* loop = loopThread.StartLoop();
    auto timer = loop->RunEvery(2, Print);
    loop->RunAfter(7, [] { Jundge(3); });
    loop->RunAfter(1, [] { Jundge(0); });
    loop->RunAfter(5, [] { Jundge(2); });
    loop->RunAfter(7, [timer, loop] { loop->Cancel(timer);});
    loop->RunAfter(9, [] { Jundge(3); });
    this_thread::sleep_for(13s);
}
