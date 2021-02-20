//
// Created by rui836662398 on 2021/2/6.
//

#ifndef MYWEBSERVER_NONCOPYABLE_H
#define MYWEBSERVER_NONCOPYABLE_H

class noncopyable {
public:
    noncopyable(const noncopyable &) = delete;

    void operator=(const noncopyable &) = delete;

protected:
    noncopyable() = default;

    ~noncopyable() = default;
};

#endif //MYWEBSERVER_NONCOPYABLE_H
