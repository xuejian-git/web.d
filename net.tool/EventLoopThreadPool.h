/**********************************************************
 *   Author          : Apriluestc
 *   Email           : 13669186256@163.com
 *   Last modified   : 2019-07-28 13:10
 *   Filename        : EventLoopThreadPool.h
 *   Description     : 
 * *******************************************************/

#ifndef INCLUDE_EVENTLOOPTHREADPOOL_H
#define INCLUDE_EVENTLOOPTHREADPOOL_H

#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>

#include "EventLoopThread.h"
#include "base/Logging.h"

class EventLoopThreadPool : boost::noncopyable {
public:
    EventLoopThreadPool(EventLoop* baseLoop, int numThreads);

    ~EventLoopThreadPool() {
        LOG << "~EventLoopThreadPool()";
    }
    void start();

    EventLoop *getNextLoop();

private:
    EventLoop* baseLoop_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::shared_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
}; // EventLoopThreadPool

#endif // INCLUDE_EVENTLOOPTHREADPOOL_H
