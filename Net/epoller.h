//
// Created by rui836662398 on 2021/2/23.
//

#ifndef MYWEBSERVER_EPOLLER_H
#define MYWEBSERVER_EPOLLER_H

#include "Net/channel.h"
#include "Utility/noncopyable.h"
#include "Utility/timestamp.h"

struct epoll_event;

class Epoller : noncopyable{
   public:
    using ChannelList = std::vector<Channel*>;

    Epoller(EventLoop* loop);
    ~Epoller();

    void UpdateChannel(Channel* channel, int operation);
    void RemoveChannel(Channel* channel);

    //active_channels will be assigned
    Timestamp Epoll(int timeout_ms, ChannelList* active_channels);

    bool HasChannel(Channel* channel);

   private:
    using ChannelMap = std::unordered_map<int, Channel*>;
    using EventList = std::vector<struct epoll_event>;

    static const int kInitEventListSize = 16;

    void FillActiveChannels(int num, ChannelList* active_channels);
    static std::string OperationToString(int operation);

    EventLoop* loop_;
    ChannelMap channels_;
    ChannelMap fd2channel_;
    int epollfd_;
    EventList events_;
};

#endif  // MYWEBSERVER_EPOLLER_H
