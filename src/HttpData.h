/**********************************************************
 *   Author          : Apriluestc
 *   Email           : 13669186256@163.com
 *   Last modified   : 2019-07-28 13:12
 *   Filename        : HttpData.h
 *   Description     : 
 * *******************************************************/

#ifndef INCLUDE_HTTPDATA_H
#define INCLUDE_HTTPDATA_H

#include <string>
#include <unordered_map>
#include <map>
#include <memory>
#include <sys/epoll.h>
#include <functional>
#include <unistd.h>
#include <vector>

#include "Timer.h"
#include "core/FastCgi.h"
#include "Util.h"

#define MAXLINE 8192

class EventLoop;
class TimerNode;
class Channel;

enum ProcessState {
    STATE_PARSE_URI = 1,
    STATE_PARSE_HEADERS,
    STATE_RECV_BODY,
    STATE_ANALYSIS,
    STATE_FINISH
};

enum URIState {
    PARSE_URI_AGAIN = 1,
    PARSE_URI_ERROR,
    PARSE_URI_SUCCESS,
};

// 解析 Header 相关成员
enum HeaderState {
    // 请求成功
    PARSE_HEADER_SUCCESS = 1,
    // 重新请求
    PARSE_HEADER_AGAIN,
    // 错误
    PARSE_HEADER_ERROR
};

enum AnalysisState {
    ANALYSIS_SUCCESS = 1,
    ANALYSIS_ERROR
};

enum ParseState {
    H_START = 0,
    H_KEY,
    H_COLON,
    H_SPACES_AFTER_COLON,
    H_VALUE,
    H_CR,
    H_LF,
    H_END_CR,
    H_END_LF
};

enum ConnectionState {
    H_CONNECTED = 0,
    H_DISCONNECTING,
    H_DISCONNECTED    
};

// 请求方法
enum HttpMethod {
    METHOD_POST = 1,
    METHOD_GET,
    METHOD_HEAD
};

// HTTP 协议版本
enum HttpVersion {
    HTTP_10 = 1,
    HTTP_11
};

class MimeType {
private:
    static void init();
    static std::unordered_map<std::string, std::string> mime;
    MimeType();
    MimeType(const MimeType &m);

public:
    // 获取请求文件的 MIME 类型
    static std::string getMime(const std::string &suffix);

private:
    static pthread_once_t once_control;
}; // MimeType


class HttpData : public std::enable_shared_from_this<HttpData> {
public:
    HttpData(EventLoop *loop, int connfd);
    ~HttpData() { close(fd_); }
    void reset();
    void seperateTimer();
    void linkTimer(std::shared_ptr<TimerNode> mtimer) {
        // shared_ptr重载了bool, 但weak_ptr没有
        timer_ = mtimer; 
    }
    std::shared_ptr<Channel> getChannel() { return channel_; }
    EventLoop *getLoop() {
        return loop_;
    }
    void handleClose();
    void newEvent();

private:
    EventLoop *loop_;
    std::shared_ptr<Channel> channel_;
    int fd_;
    std::string inBuffer_;
    std::string outBuffer_;
    // std::vector<char> inBuffer_;
    // std::vector<char> outBuffer_;
    bool error_;
    ConnectionState connectionState_;

    HttpMethod method_;
    HttpVersion HTTPVersion_;
    std::string fileName_;
    std::string path_;
    int nowReadPos_;
    ProcessState state_;
    ParseState hState_;

    // 长连接标志
    bool keepAlive_;
    std::map<std::string, std::string> headers_;
    std::weak_ptr<TimerNode> timer_;

    // 读处理
    void handleRead();

    // 写处理
    void handleWrite();

    // 处理连接(包括 current connection and next connection)
    void handleConn();
    void handleError(int fd, int err_num, std::string short_msg);
    URIState parseURI();
    HeaderState parseHeaders();
    AnalysisState analysisRequest();

public:
    // 动态文件请求处理
    static void handleDynamic();

    // 将 php 相应结果发送给浏览器客户端回调
    static int sendPhpToCli(int fd, int outlen, char* out, int errlen, char* err, FastCgiEndRequestBody* endr);

    // 发送 HTTP 请求给 FastCgi 服务器
    // static int sendFastCgi(rio_t* rp, hhr_t* hp, int sock);

    // 接收 FastCgi 返回的数据
    static int recvFastCgi(int fd, int fcsock);

}; // HttpData

#endif // INCLUDE_HTTPDATA_H
