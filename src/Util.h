/**********************************************************
 *   Author          : Apriluestc
 *   Email           : 13669186256@163.com
 *   Last modified   : 2019-07-28 13:16
 *   Filename        : Util.h
 *   Description     : 
 * *******************************************************/

#ifndef INCLUDE_UTIL_H
#define INCLUDE_UTIL_H

#include <cstdlib>
#include <string>

#define RIO_BUFSIZE 8192
struct rio_t {
    // 内部缓冲区对应的描述符
    int rio_fd;

    // 可以读取的字节数
    int rio_cnt;

    // 下一个可以读取的字节地址
    char *rio_bufptr;

    // 内部缓冲区
    char rio_buf[RIO_BUFSIZE];
};

ssize_t readn(int fd, void *buff, size_t n);
ssize_t readn(int fd, std::string &inBuffer, bool &zero);
ssize_t readn(int fd, std::string &inBuffer);
ssize_t writen(int fd, void *buff, size_t n);
ssize_t writen(int fd, std::string &sbuff);
void handle_for_sigpipe();
int setSocketNonBlocking(int fd);
void setSocketNodelay(int fd);
void setSocketNoLinger(int fd);
void shutDownWR(int fd);
int socket_bind_listen(int port);
ssize_t rio_readn(int fd, void *usrbuf, size_t n);                         
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);

#endif // INCLUDE_UTIL_H
