/**********************************************************
 *   Author          : Apriluestc
 *   Email           : 13669186256@163.com
 *   Last modified   : 2019-08-04 10:40
 *   Filename        : FastCgi.h
 *   Description     : 
 * *******************************************************/

#ifndef INCLUDE_FASTCGI_H
#define INCLUDE_FASTCGI_H

#include <functional>

#define FASTCGI_MAX_LENGTH 65535
// #define FASTCGI_MAX_LENGTH 0xffff
#define DEFAULT_FASTCGI_ADDR "127.0.0.1"
#define DEFAULT_FASTCGI_PORT 9000
#define FASTCGI_VERSION 1

// FASTCGI 协议包头长度
#define FASTCGI_HEADER_LENGTH 8

#define FASTCGI_KEEP_CONN  1

// FASTCGI 协议组件的类型值
enum HandleState {
    
    // 请求开始记录类型
    FASTCGI_BEGIN_REQUEST = 1,
    FASTCGI_ABORT_REQUEST,
    
    // 响应结束记录类型
    FASTCGI_END_REQUEST,
    
    // 传输明值键值对
    FASTCGI_PARAMS,
    
    // 传输输入数据，例如 POST 数据
    FASTCGI_STDIN,
    
    // 数据响应输出
    FASTCGI_STDOUT,
    
    // 错误输出
    FASTCGI_STDERR,
    FASTCGI_DATA
};

// 协议级别状态码
enum VersionState {
    
    // 正常结束
    FASTCGI_REQUEST_COMPLETE = 1,
    
    // 拒绝新请求，无法并发处理
    FASTCGI_CANT_MPX_CONN,
    
    // 拒绝新请求，资源负载
    FASTCGI_OVERLOADED,
    
    // 不能识别的角色
    FASTCGI_UNKNOWN_ROLE
};

// php-fpm 角色值
enum RoleState {
    FASTCGI_RESPONDER = 1,
    FASTCGI_AUTHORIZER,
    FASTCGI_FILTER,
};

// FASTCGI 协议报头
struct FastCgiHeader {
    
    // 协议版本
    unsigned char version;
    
    // 协议记录类型
    unsigned char type;
    
    // 请求 ID
    unsigned char requestIdB1;
    unsigned char requestIdB0;
    
    // 内容长度
    unsigned char contentLengthB1;
    unsigned char contentLengthB0;
    
    // 填充字节长度
    unsigned char paddingLength;
    
    // 保留字节
    unsigned char reserved;
};

// FASTCGI 请求开始记录的协议结构
struct FastCgiBeginRequestBody {
    
    // Web 服务器期望 php 所扮演的角色
    unsigned char roleB1;
    unsigned char roleB0;
    
    // 控制连接响应后是否立即关闭
    unsigned char flag;
    unsigned char reserved[5];
};

// 开始请求记录结构，包含开始请求协议头和协议体
struct FastCgiBeginRequestRecord {
    FastCgiHeader header;
    FastCgiBeginRequestBody body;
};

// 结束请求记录的协议结构
struct FastCgiEndRequestBody {
    unsigned char appStatusB3;
    unsigned char appStatusB2;
    unsigned char appStatusB1;
    unsigned char appStatusB0;
    unsigned char protocolStatus;
    unsigned char reserved[3];
};

// 结束请求记录结构
struct FastCgiEndRequestRecord {
    FastCgiHeader header;
    FastCgiEndRequestBody body;
};

struct FastCgiParamRecord {
    FastCgiHeader header;
    unsigned char namelength;
    unsigned char valuelength;
    unsigned char data[0];
};

class FastCgiData {
public:
    
    // 读取发送协议记录函数指针回调
    typedef std::function<size_t(int, void*, size_t)> CallFunc;
    
    // 发送 php 结果给浏览器客户端函数回调
    typedef std::function<size_t(int, int, char*, int, char*, FastCgiEndRequestBody*)> CallCli;
    FastCgiData() {}
    ~FastCgiData() {}

    // 构造协议 Header
    FastCgiHeader MakeHeader(int type, int request, int contentlength, int paddinglength);
    
    // 构造开始请求记录协议
    FastCgiBeginRequestBody MakeBeginRequestBody(int role, int KeepConn);
    
    // 发送开始请求记录
    int SendBeginRequestRecord(CallFunc cf, int fd, int requestId);
    
    // 发送明值对参数
    int SendParamRecord(CallFunc cf, int fd, int requestId, char* name, int nlen, char* value, int vlen);
    
    // 发送空的 Params 记录
    int SendEmptyParamsRecord(CallFunc cf, int fd, int requestId);
    
    // 发送 FastCgiStdin 数据
    int SendStdinRecord(CallFunc cf, int fd, int requestId, char* data, int len);
    
    // 发送空的 FastCgiStdin 记录
    int SendEmptyStdinRecord(CallFunc cf, int fd, int requestId);
    
    // 接收记录，处理
    int RecvRecord(CallFunc cf, CallCli stc, int cfd, int fd, int requestId);

}; // FastCgiData

#endif // INCLUDE_FASTCGI_H
