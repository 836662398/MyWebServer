//
// Created by rui836662398 on 2021/2/23.
//

#include "epoller.h"

#include <sys/epoll.h>
#include <unistd.h>

#include "Utility/logging.h"

// for logging
static std::string unit_name = "Epoller";

Epoller::Epoller(EventLoop *loop) : loop_(loop), events_(kInitEventListSize) {
    epollfd_ = ::epoll_create1(EPOLL_CLOEXEC);
    if (epollfd_ < 0) {
        FATAL("Failed to epoll_create1!");
    }
}

Epoller::~Epoller() { ::close(epollfd_); }

void Epoller::UpdateChannel(Channel *channel, int operation) {
    struct epoll_event event;
    memset(&event, 0, sizeof event);
    event.events = channel->events();
    // Channel* is stored in data.ptr
    event.data.ptr = channel;
    int fd = channel->fd();
    if (operation == EPOLL_CTL_ADD) {
        assert(fd2channel_.find(fd) == fd2channel_.end());
        fd2channel_[fd] = channel;
    } else if (operation == EPOLL_CTL_MOD)
        assert(fd2channel_.find(fd) != fd2channel_.end());
    else if (operation == EPOLL_CTL_DEL) {
        assert(fd2channel_.find(fd) != fd2channel_.end());
        fd2channel_.erase(fd);
    }
    TRACE(fmt::format("Operation: {}, {}", OperationToString(operation),
                      channel->EventsToString()));
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        ERROR(fmt::format("Failed to epoll_ctl, operation: {}, {}",
                          OperationToString(operation),
                          channel->EventsToString()));
    }
}

void Epoller::RemoveChannel(Channel *channel) {
    int fd = channel->fd();
    if (fd2channel_.find(fd) != fd2channel_.end()) {
        UpdateChannel(channel, EPOLL_CTL_DEL);
    }
}

Timestamp Epoller::Epoll(int timeout_ms,
                         Epoller::ChannelList *active_channels) {
    int events_num =
        ::epoll_wait(epollfd_, events_.data(), events_.size(), timeout_ms);
    int saved_errno = errno;
    auto now = Timestamp::Now();
    if (events_num > 0) {
        TRACE(fmt::format("{} events happened.", events_num));
        FillActiveChannels(events_num, active_channels);
        // size extends if necessary
        if (events_num == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (events_num == 0)
        TRACE("Nothing happened.");
    else {
        if (saved_errno != EINTR) {
            errno = saved_errno;
            ERROR("epoll_wait() failed!");
        }
    }
    return now;
}

void Epoller::FillActiveChannels(int num,
                                 Epoller::ChannelList *active_channels) {
    assert(static_cast<size_t>(num) <= events_.size());
    for (int i = 0; i < num; i++) {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        assert(fd2channel_[channel->fd()] == channel);
        channel->set_revents(events_[i].events);
        active_channels->push_back(channel);
    }
}

std::string Epoller::OperationToString(int operation) {
    switch (operation) {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "False operation.");
            return "Unknown Operation";
    }
}

bool Epoller::HasChannel(Channel *channel) {
    int fd = channel->fd();
    auto it = fd2channel_.find(fd);
    return it != fd2channel_.end() && it->second == channel;
}
