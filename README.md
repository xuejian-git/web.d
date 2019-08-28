# WebServer

[![](https://img.shields.io/badge/build-pass-brightgreen)](https://github.com/Apriluestc/web.d/edit/master/README.md)

**测试页：**[Apriluestc's.pub](http://39.107.70.253:20000/)

## 项目介绍

本项目为 C++ 11 编写的高性能 Web 服务器，解析了 GET、POST 请求，可静态处理资源，支持 HTTP 长连接、支持
管线化请求，并实现了异步日志，实时记录服务器运行状态

## 技术特点

- 程序使用 Epoll 边沿触发、非阻塞 IO、Reactor 模式
- 使用多线程技术充分发挥多核 CPU 性能，并使用线程池规避线程频繁创建销毁的开销
- 使用基于时间轮的定时器关闭超时请求和剔除不活跃连接
- 主线程负责 accept 请求，并轮询分发 fd 给其他 IO 线程，这样一来锁竞争只会出现在主线程和某一特定线程
- 使用 eventfd 跨线程异步唤醒
- 使用 TCP 环形缓冲区，减少 read、write 等系统调用的次数，进而减少系统开销
- 使用智能指针、RAII 机制规避程序中出现内存泄漏的可能
- 实现异步日志实时记录服务器运行状态(便于 Debug)
- webd 服务以 dameon 进程运行

## 职责

首先，本项目分为 3 个模块，分别是  HTTP、事件驱动、日志

**我负责实现日志模块和事件驱动模块**

- 事件驱动模块

对于事件驱动模块包含两个核心类 Channel、EventLoop，其中，Channel 是 Reactor 模式中的事件，自始至终只属于一个 EventLoop，负责一个文件描述符的 IO 事件，
在 Channel 类中保存这些 IO 事件的类型及与其对应的 CallBackFunc，当 IO 事件发生的时候，最终会调用到 Channel 中的回调函数，因此，程序中
所有带有读写事件的对象都会和一个 Channel 关联，包括 Loop 中的 eventfd、listenfd，EventLoop 中意味着每个线程只能有一个 EventLoop 对象，EventLoop 即是
时间循环，每次从 Epoller 中拿活跃事件，并给到 Channel 里分发处理，EventLoop 中的 Loop 函数会在最底层 Thread 中调用，开始轮询，直到某一轮检测到退出状态后，才逐层退出

- 日志模块

首先多线程异步日志需要线程安全的保证，即多个线程可以写日志文件而不发生错乱，简单的线程安全并不难办到，用一个 全局的 Mutex 对日志的 IO 操作进行保护或者
单独写一个日志文件即可，但是前者会造成多个线程竞争锁资源，后者会造成某个业务线程阻塞

本项目可行的解决方案就是，用一个背景线程负责收集日志消息并将其写入后端，其他业务线程只负责生成的日志消息并将其传输到日志线程，这便是异步日志

日志的实现应用双缓冲区技术，即存在两个 Buffer，日志的实现分为前端和后端， 前端负责向 CurrentBuffer 中写，后端负责 将其写入文件中，具体来说，当 CurrentBuffer 写满
时，先将 CurrentBuffer 中的消息存入 Buffer 中，在交换 CurrentBuffer 和 NextBuffer，这样 前端就可以继续往 CurrentBuffer 中写新的日志
消息， 最后再调用 notify_all 通知后端将其写入文件

## 项目来源

- [陈硕](https://github.com/chenshuo/muduo/tree/master/muduo/net)
- 林亚
- 陈帅豪
