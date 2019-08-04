# WebServer (学习项目)

[![](https://img.shields.io/badge/build-pass-brightgreen)](https://github.com/Apriluestc/web.d/edit/master/README.md)

## 项目描述

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
本项目为 C++11 编写的 Web 服务器，解析了 get、head 请求，可处理静态资源，动态资源，支
持HTTP长连接，支持管线化请求，并实现了异步日志，记录服务器运行状态

## 项目目的

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
该项目用作秋招复习自己所学知识，其涵盖了：
- 多线程多进程编程、socket 编程、泛型编程
- TCP/IP、HTTP 协议
- IO 相关(包括标准 IO、IO 设计模式、五大 IO 模型)
- 多线程相关竞态同步原语
- dameon 进程创建原理及工作原理
- 版本控制工具 git，调试工具 gdb，构建工具 make
- shell、编程规范

做完这个项目可以把所学知识复习一遍，哈哈哈哈哈哈迫不及待

后续项目中进行重构，采用智能指针代替全裸型指针规避内存泄漏的可能，借鉴 muduo 思想，林雅写的项目以及陈帅豪写
的项目，将 webd 重构了一番

项目中使用 shell 脚本，编写自动化服务控制脚本，涵盖了 shell 语法中的基本变量、循环控制语句、linux 下信号控制

## 技术特点

- 使用 Epoll 边沿触发的 IO 多路复用技术，非阻塞 IO，使用 Reactor 模式
- 使用多线程充分利用多核 CPU，并使用线程池避免线程频繁创建销毁的开销
- 使用基于小根堆的定时器关闭超时请求
- 使用状态机解析了 HTTP GET 和 HEAD 请求
- 主线程只负责 accept 请求，并以 Round Robin 的方式分发给其它 IO 线程(兼计算线程)，
锁的争用只会出现在主线程和某一特定线程中
- 使用保活机制实现了 HTTP 长连接
- 使用环形缓冲区作为 TCP 的接收发送缓冲区
- 使用 eventfd 实现线程的异步唤醒
- 使用双缓冲区技术实现了简单的异步日志系统
- 为减少内存泄漏的可能，使用智能指针等 RAII 机制
- 支持优雅关闭连接

## 项目来源

- [陈硕](https://github.com/chenshuo/muduo/tree/master/muduo/net)
- 林亚
- 陈帅豪

## 代码统计

![Add image](https://github.com/Apriluestc/web.d/blob/master/image/num.png)

## 职责 (我实现了)

- 实现了基于时间轮的定时器，定时关闭超时请求以及剔除不活跃连接
- 实现了线程安全的高性能内存池 MemoryPool，减少频繁开辟和释放产生的内存碎片
- 解析了 HTTP POST 请求
- 实现了 TCP 接收发送缓冲区(环形缓冲区)
- 比较本项目与其他 Web 服务器的并发模型特点，如：nginx
- 使用 shell 脚本编写了 webd 服务的启动、关闭及重启(控制服务行为)
- 该服务以 dameon 进程运行
- 动态文件请求处理(正在实现)于 core 目录下

## 使用指南

```bash
git clone https://github.com/Apriluestc/web.d.git
cd web.d
make
make common
make debug
```
