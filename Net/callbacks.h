//
// Created by rui836662398 on 2021/3/1.
//

#ifndef MYWEBSERVER_CALLBACKS_H
#define MYWEBSERVER_CALLBACKS_H

#include <functional>

class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
// use TcpConnectionPtr as parameter to store TcpConnection objects for RAII
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*)>;

#endif  // MYWEBSERVER_CALLBACKS_H
