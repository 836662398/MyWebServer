//
// Created by rui836662398 on 2021/2/20.
//

#ifndef MYWEBSERVER_CHANNEL_H
#define MYWEBSERVER_CHANNEL_H

#include <sys/epoll.h>

#include <functional>

#include "Utility/noncopyable.h"

class EventLoop;

class Channel : noncopyable {
   public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void set_read_callback(const EventCallback& cb) { read_callback_ = cb; }
    void set_write_callback(const EventCallback& cb) { write_callback_ = cb; }
    void set_close_callback(const EventCallback& cb) { close_callback_ = cb; }
    void set_error_callback(const EventCallback& cb) { error_callback_ = cb; }
    void HandleEvent();

    int fd() { return fd_; }
    int events() { return events_; }
    int set_revents(int revent) { revents_ = revent; }
    EventLoop* loop() { return loop_; }

    void ETInit();
    void ETEnableReading();
    void EnableReading();
    void EnableWriting();
    void DisableReading();
    void DisableWriting();
    void DisableAll();
    bool IsReading() { return events_ & kReadEvent; }
    bool IsWriting() { return events_ & kWriteEvent; }

    void Remove();

    // for debug
    std::string EventsToString() const;
    std::string ReventsToString() const;

   private:
    static const int kNoneEvent = 0;
    static const int kReadEvent = EPOLLIN | EPOLLPRI;
    static const int kWriteEvent = EPOLLOUT;
    static const int kETEvent = EPOLLIN | EPOLLPRI | EPOLLOUT | EPOLLET;

    void Update(int operation);
    static std::string EventsToString(int fd, int ev);

    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_;

    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;
};

#endif  // MYWEBSERVER_CHANNEL_H
