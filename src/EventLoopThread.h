/**********************************************************
 *   Author          : Apriluestc
 *   Email           : 13669186256@163.com
 *   Last modified   : 2019-07-28 13:09
 *   Filename        : EventLoopThread.h
 *   Description     : 
 * *******************************************************/

#ifndef INCLUDE_EVENTLOOPTHREAD_H
#define INCLUDE_EVENTLOOPTHREAD_H

#include <boost/noncopyable.hpp>

#include "base/Condition.h"
#include "base/MutexLock.h"
#include "base/Thread.h"
#include "EventLoop.h"

class EventLoopThread : boost::noncopyable {
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();
    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
}; // EventLoop

#endif // INCLUDE_EVENTLOOPTHREAD_H
