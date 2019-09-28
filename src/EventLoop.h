/**********************************************************
 *   Author          : Apriluestc
 *   Email           : 13669186256@163.com
 *   Last modified   : 2019-07-28 13:08
 *   Filename        : EventLoop.h
 *   Description     : IO 复用流程的数据抽象
 *   等待事件、处理事件、执行其他业务
 *   事件循环，一直查找新的事件并且执行，一次循环的执行称为 tick，
 *
 *   循环体中的代码称为 task
 *
 *   EventLoop 是 Reactor 模式的核心，一个线程对应一个事件循环，他的生命周期和
 *   所属线程一样，它主要负责在循环中等待各类事件的触发，然后调用对应 Channel，每一个事件对应一个 Channel
 *   
 *   任何一个线程只要创建并运行了EventLoop,就是一个所谓的IO线程, 如果我在主线程里调用EventLoopThread创建一个子线程, 在子线程中创建一个EventLoop对象, 并在主线程中拿到了该EventLoop对象的指针, 然后在主线程调用EventLoop::runInLoop, 那么就不是在IO线程中, 就会调用EventLoop::queueInLoop添加任务到IO线程的任务队列, 然后唤醒IO线程, 执行任务
 * *******************************************************/

#ifndef INCLUDE_EVENTLOOP_H
#define INCLUDE_EVENTLOOP_H

#include <vector>
#include <memory>
#include <functional>
#include <iostream>

#include "base/Thread.h"
#include "Epoll.h"
#include "base/Logging.h"
#include "Channel.h"
#include "base/CurrentThread.h"
#include "Util.h"

class EventLoop {
public:
    // 回调函数类型，及业务类型
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();
    
    // 该函数循环调用 poll 函数
    // 该函数返回更新的就绪事件列表
    // 然后再遍历就绪事件列表 vector<Channel*> 
    // 再调用 handleEvent 函数进行事件的分类处理
    // 真正的事件逻辑分类是 handleEvent
    void loop();
    void quit();

    // runInLoop 这个函数判断如果当前线程是不是 IO 线程，如果是就直接执行任务，
    // 否则就添加至任务队列中，并唤醒 IO 线程，让他执行任务
    // 这里在唤醒的时候用到了 eventfd 
    // eventfd 是 Linux 2.6.22 后系统提供的一个轻量级进程间通信的系统调用，可以
    // 进行多进程多线程之间的事件通知，eventfd 通过一个进程间共享 64 位计数器完成进程
    // 间通信，这个计数器由内核维护用户可以通过屌用 write 方法像内核空间写入一个 64 位的值，也可以调用
    // read 方法读取这个值
    //
    // eventfd 实现了线程之间事件通知的方式，可用于内核态和内核通信，eventfd 的缓冲区大小是
    // sizeof(uint64_t)，向其 write 可以递增这个计数器，read 操作可以读取也可以清零，eventfd 也可以放到监听队列中，当计数器不是 0 时，
    // 有可读事件发生，可进行读取
    void runInLoop(Functor&& cb);
    void queueInLoop(Functor&& cb);
    bool isInLoopThread() const {
        return threadId_ == CurrentThread::tid();
    }
    void assertInLoopThread() {
        assert(isInLoopThread());
    }

    // 关闭文件描述符
    void shutdown(std::shared_ptr<Channel> channel) {
        shutDownWR(channel->getFd());
    }

    // 移除事件
    void removeFromPoller(std::shared_ptr<Channel> channel) {
        //shutDownWR(channel->getFd());
        poller_->epoll_del(channel);
    }

    // 修改事件
    void updatePoller(std::shared_ptr<Channel> channel, int timeout = 0) {
        poller_->epoll_mod(channel, timeout);
    }

    // 添加事件
    void addToPoller(std::shared_ptr<Channel> channel, int timeout = 0) {
        poller_->epoll_add(channel, timeout);
    }
    
private:
    // 声明顺序 wakeupFd_ > pwakeupChannel_
    bool looping_;

    // epoll 对象
    std::shared_ptr<Epoll> poller_;

    // 跨线程唤醒 fd
    int wakeupFd_;

    // 线程运行状态
    bool quit_;
    bool eventHandling_;

    // 保护任务列表的互斥量
    mutable MutexLock mutex_;

    // 任务列表
    std::vector<Functor> pendingFunctors_;
    bool callingPendingFunctors_;

    // loop 所在线程 id
    const pid_t threadId_; 

    // 事件列表类型
    std::shared_ptr<Channel> pwakeupChannel_;
    
    // 唤醒 loop
    void wakeup();

    // 唤醒 loop 的读事件回调
    void handleRead();
    void doPendingFunctors();

    // 唤醒 loop 后的错误事件回调
    void handleConn();
}; // EventLoop

#endif // INCLUDE_EVENTLOOP_H
