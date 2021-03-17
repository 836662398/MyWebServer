//
// Created by rui836662398 on 2021/2/20.
//

#include "channel.h"

#include <sstream>

#include "Net/event_loop.h"
#include "Utility/logging.h"

// for logging
static std::string unit_name = "Channel";

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0) {}

Channel::~Channel() {
    if (loop_->IsInLoopThread()) {
        assert(!loop_->HasChannel(this));
    }
}

void Channel::HandleEvent() {
    if (revents_ & EPOLLHUP && !(revents_ & EPOLLIN)) {
        if (close_callback_) close_callback_();
        ERROR("EPOLLHUP!");
        return;
    }
    if (revents_ & (EPOLLERR)) {
        if (error_callback_) error_callback_();
        ERROR("EPOLLERR!");
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (read_callback_) read_callback_();
    }
    if (revents_ & EPOLLOUT) {
        if (write_callback_) write_callback_();
    }
}

void Channel::EnableReading() {
    if (IsReading()) return;
    auto op = (events_ == kNoneEvent ? EPOLL_CTL_ADD : EPOLL_CTL_MOD);
    events_ |= kReadEvent;
    Update(op);
}
void Channel::EnableWriting() {
    if (IsWriting()) return;
    auto op = (events_ == kNoneEvent ? EPOLL_CTL_ADD : EPOLL_CTL_MOD);
    events_ |= kWriteEvent;
    Update(op);
}
void Channel::DisableReading() {
    if (!IsReading()) return;
    events_ &= ~kReadEvent;
    auto op = (events_ == kNoneEvent ? EPOLL_CTL_DEL : EPOLL_CTL_MOD);
    Update(op);
}
void Channel::DisableWriting() {
    if (!IsWriting()) return;
    events_ &= ~kWriteEvent;
    auto op = (events_ == kNoneEvent ? EPOLL_CTL_DEL : EPOLL_CTL_MOD);
    Update(op);
}
void Channel::DisableAll() {
    if (events_ == kNoneEvent) return;
    events_ = kNoneEvent;
    Update(EPOLL_CTL_DEL);
}

void Channel::Remove() { loop_->RemoveChannel(this); }
void Channel::Update(int operation) { loop_->UpdateChannel(this, operation); }

std::string Channel::ReventsToString() const {
    return EventsToString(fd_, revents_);
}

std::string Channel::EventsToString() const {
    return EventsToString(fd_, events_);
}

std::string Channel::EventsToString(int fd, int ev) {
    std::ostringstream oss;
    oss << "fd " << fd << ": ";
    if (ev & EPOLLIN) oss << "IN ";
    if (ev & EPOLLPRI) oss << "PRI ";
    if (ev & EPOLLOUT) oss << "OUT ";
    if (ev & EPOLLHUP) oss << "HUP ";
    if (ev & EPOLLRDHUP) oss << "RDHUP ";
    if (ev & EPOLLERR) oss << "ERR ";

    return oss.str();
}
