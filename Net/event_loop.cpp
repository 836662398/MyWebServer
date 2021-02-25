//
// Created by rui836662398 on 2021/2/24.
//

#include "event_loop.h"

#include <signal.h>
#include <sys/eventfd.h>

#include "Net/channel.h"
#include "Net/epoller.h"
#include "Utility/logging.h"
#include "Utility/types.h"

static std::string unit_name = "EventLoop";

namespace {
thread_local EventLoop *thread_local_eventloop = 0;

class IgnoreSigPipe {
   public:
    IgnoreSigPipe() { ::signal(SIGPIPE, SIG_IGN); }
};
IgnoreSigPipe init_sig;

}  // namespace

EventLoop::EventLoop()
    : looping_(false),
      thread_id_(std::this_thread::get_id()),
      quit_flag_(false),
      epoller_(std::make_unique<Epoller>(this)) {
    wakeup_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeup_fd_ < 0) FATAL("Failed to create eventfd!");
    if (!thread_local_eventloop)
        thread_local_eventloop = this;
    else
        FATAL(fmt::format("Another EventLoop has existed in this thread {}.",
                          IdToInt(thread_id_)));
    DEBUG(fmt::format("EventLoop created in thread {}", IdToInt(thread_id_)));
    wakeup_channel_->set_read_callback_(
        std::bind(&EventLoop::WakeUpReadHandle, this));
    wakeup_channel_->EnableReading();
}

EventLoop::~EventLoop() {
    wakeup_channel_->DisableAll();
    wakeup_channel_->Remove();
    ::close(wakeup_fd_);
    thread_local_eventloop = nullptr;
}

void EventLoop::Loop() {
    assert(!looping_);
    AssertInLoopThread();
    // if someone Quit() before Loop()
    quit_flag_ = false;
    looping_ = true;
    TRACE(fmt::format("EventLoop in thread {} start looping.",
                      IdToInt(thread_id_)));
    while (!quit_flag_) {
        active_channels_.clear();
        epoller_->Epoll(kEpollTimeoutMs, &active_channels_);
        for (Channel *channel : active_channels_) {
            channel->HandleEvent();
        }
        HandlePendingCallbacks();
    }
    TRACE(fmt::format("EventLoop in thread {} stop looping.",
                      IdToInt(thread_id_)));
    looping_ = false;
}

void EventLoop::Quit() {
    quit_flag_ = false;
    if (!IsInLoopThread()) {
        WakeUp();
    }
    // Otherwise next cycle starts a little while
}

void EventLoop::RunInLoop(EventLoop::Callback cb) {
    if (IsInLoopThread())
        cb();
    else
        QueueInLoop(cb);
}

void EventLoop::QueueInLoop(EventLoop::Callback cb) {
    {
        std::lock_guard<std::mutex> lck(mutex_);
        pending_callbacks_.push_back(cb);
    }

    if (!IsInLoopThread() || pending_callbacks_handling_) {
        WakeUp();
    }
    // Otherwise HandlePendingCallbacks() starts a little while
}

void EventLoop::WakeUp() {
    uint64_t num = 1;
    ssize_t n = ::write(wakeup_fd_, &num, sizeof num);
    if (n != 8) {
        ERROR(fmt::format("WakeUp() writed {} bytes instead of 8!", n));
    }
}

void EventLoop::UpdateChannel(Channel *channel, int operation) {
    assert(channel->loop() == this);
    AssertInLoopThread();
    epoller_->UpdateChannel(channel, operation);
}
void EventLoop::RemoveChannel(Channel *channel) {
    assert(channel->loop() == this);
    AssertInLoopThread();
    epoller_->RemoveChannel(channel);
}
bool EventLoop::HasChannel(Channel *channel) {
    assert(channel->loop() == this);
    AssertInLoopThread();
    epoller_->HasChannel(channel);
}
bool EventLoop::IsInLoopThread() {
    return std::this_thread::get_id() == thread_id_;
}
void EventLoop::AssertInLoopThread() {
    if (!IsInLoopThread()) {
        FATAL(fmt::format(
            "EventLoop was created in thread {}, the current thread id is {}",
            IdToInt(thread_id_), IdToInt(std::this_thread::get_id())));
    }
}

EventLoop *EventLoop::get_thread_local_eventloop() {
    return thread_local_eventloop;
}

void EventLoop::WakeUpReadHandle() {
    uint64_t buf;
    ssize_t n = ::read(wakeup_fd_, &buf, sizeof buf);
    if (n != 8)
        ERROR(fmt::format("WakeUpReadHandle() read {} bytes instead of 8!", n));
}
void EventLoop::HandlePendingCallbacks() {
    std::vector<Callback> callbacks;
    pending_callbacks_handling_ = true;
    {
        std::lock_guard<std::mutex> lck(mutex_);
        pending_callbacks_.swap(callbacks);
    }
    for (auto &cb : callbacks) cb();
    pending_callbacks_handling_ = false;
}
