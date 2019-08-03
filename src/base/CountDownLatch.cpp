/**********************************************************
 *   Author          : Apriluestc
 *   Email           : 13669186256@163.com
 *   Last modified   : 2019-07-28 13:29
 *   Filename        : CountDownLatch.cpp
 *   Description     : 
 * *******************************************************/

#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count)
    :mutex_(),
    condition_(mutex_),
    count_(count)
{}

void CountDownLatch::wait() {
    // std::lock_guard<std::mutex> lock(mutex_);
    MutexLockGuard lock(mutex_);
    while (count_ > 0)
        condition_.wait();
}

void CountDownLatch::countDown() {
    // std::lock_guard<std::mutex> lock(mutex_);
    MutexLockGuard lock(mutex_);
    --count_;
    if (count_ == 0)
        condition_.notifyAll();
}
