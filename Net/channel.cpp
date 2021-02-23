//
// Created by rui836662398 on 2021/2/20.
//

#include "channel.h"

#include "Utility/logging.h"

// for logging
static std::string unit_name = "Channel";

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0) {}

void Channel::HandleEvent() {
    if (revents_ & EPOLLHUP) {
        if (close_callback_) close_callback_();
        ERROR("EPOLLHUP!");
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

void Channel::Remove() {
    //TODO
    //loop_->RemoveChannel(this);
}
void Channel::Update() {
    //TODO
    //loop_->UpdateChannel(this);
}
