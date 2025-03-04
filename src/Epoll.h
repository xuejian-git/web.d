/**********************************************************
 *   Author          : Apriluestc
 *   Email           : 13669186256@163.com
 *   Last modified   : 2019-07-28 13:07
 *   Filename        : Epoll.h
 *   Description     : 
 *
 *
 * *******************************************************/

#ifndef INCLUDE_EPOLL_H
#define INCLUDE_EPOLL_H

#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include <memory>

#include "Channel.h"
#include "HttpData.h"
#include "Timer.h"

class Epoll {
public:
    Epoll();
    ~Epoll();

    /*
     * epoll_create
     * epoll_ctl
     * epoll_wait
     * */
    void epoll_add(SP_Channel request, int timeout);
    void epoll_mod(SP_Channel request, int timeout);
    void epoll_del(SP_Channel request);
    std::vector<std::shared_ptr<Channel>> poll();
    std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_num);
    void add_timer(std::shared_ptr<Channel> request_data, int timeout);
    int getEpollFd() {
        return epollFd_;
    }
    void handleExpired();
private:
    static const int MAXFDS = 100000;
    int epollFd_;
    std::vector<epoll_event> events_;
    std::shared_ptr<Channel> fd2chan_[MAXFDS];
    std::shared_ptr<HttpData> fd2http_[MAXFDS];
    TimerManager timerManager_;
}; // Epoll

#endif // INCLUDE_EPOLL_H
