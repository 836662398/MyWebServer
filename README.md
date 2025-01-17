# MyWebServer

## Introduction

基于[muduo](https://github.com/chenshuo/muduo)网络库的C++ 高性能简单静态web服务器。

对muduo进行了一些优化：

1. 允许踢除空闲连接；

2. 尽量使用C++标准库，减少了代码量；

3. 简化了定时器实现，使用堆而不是基于红黑树的set；

4. 分别实现 epoll ET/LT两种模式（位于不同branch）；

   ......

此外，日志模块使用了[spdlog](https://github.com/gabime/spdlog)库，单元测试使用了[gtest](https://github.com/google/googletest)库。

## Envoirment

- OS：Ubuntu18.04
- Complier: g++ 7.5.0
- Memory：8G
- CPU：i5 10400
- Thread Number：12

## Build

```
./build.sh	#默认生成debug版本
```

如果想要运行单元测试，需要先安装gtest：https://github.com/google/googletest/blob/master/googletest/README.md

如果未安装spdlog库，则使用header-only形式，会导致编译较慢。安装spdlog静态库，可以大大加快编译速度。

## Usage

```
./MyWebServer [-s thread_numbers] [-b] [-p process_numbers] [-h heartbeat_seconds]
```

-b：benchmark模式，只打印 ERROR/FATAL 级别日志。

-s：开启sub线程数量，不包括主线程，默认数量 4。

-p：开启进程数，大于1时，fork()多个进程监听端口，负载均衡。其中每一个进程都有 thread_numbers 个sub线程。默认单进程。

-h：开启空闲踢除，超时时间为 heartbeat_seconds 。

## Technical points

-  Muti-reactor模式，使用 非阻塞IO + IO复用，分别实现了epoll LT/ET两种模式的网络库。
-  使用了多线程，每个线程一个reactor，主线程仅负责accept连接，将建立的新连接分发给其他线程，使用 Round Robin 算法进行调度。
-  一个线程负责多个连接，每个连接仅被一个线程负责。
-  一个连接上的操作如果位于其他线程，可以通过函数调用的形式排队放进负责该连接的IO线程，以此在不用锁的情况下保证线程安全性 。
-  添加了空闲踢除功能，使用基于小根堆和 timerfd 的定时器关闭超时请求。
-   使用 eventfd 实现了线程的异步唤醒 。
-  使用智能指针等RAII技术，尽量避免使用 new/delete，防止内存泄漏。
-  使用自动状态机解析HTTP请求。

##  Concurrency Model

 ![img](https://github.com/836662398/MyWebServer/blob/ET/pic.assets/model.png) 

特点是one loop per thread，Base IO Thread负责accept(2)新连接，然后把新连接挂在某个sub IO Thread中（采用round-robin 轮询调度的方式来选择sub Reactor）。 线程数固定，IO Threads在进程初始化时创建。

此外，允许fork()多个进程。多个进程共同监听端口，负载均衡。

## Performance Comparison

| QPS    | MyWebServer_ET | MyWebServer_LT | muduo  |
| ------ | -------------- | -------------- | ------ |
| 单进程 | 92 031         | 92 055         | 91 193 |
| 双进程 | 97 054         | 97 247         |        |

单进程开启 5 个 Sub IO Thread，双进程每个进程开启3个 Sub IO Thread。

[详细记录](https://github.com/836662398/MyWebServer/blob/ET/TestRecord.md)
