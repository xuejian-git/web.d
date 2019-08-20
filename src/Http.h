/**********************************************************
 *   Author          : Apriluestc
 *   Email           : 13669186256@163.com
 *   Last modified   : 2019-08-20 10:46
 *   Filename        : Public.h
 *   Description     : 
 * *******************************************************/

#ifndef INCLUDE_PUBLIC_H
#define INCLUDE_PUBLIC_H

#include <iostream>
#include <map>
#include <functional>
#include <mutex>
#include <regex>
#include <vector>
#include <sys/socket.h>

typedef int socket_t;
typedef std::multimap<std::string, std::string> Params;
typedef std::smatch Match;
typedef std::function<bool(uint64_t current, uint64_t total)> Progress;

struct ci {
    bool operator()(const std::string &s1, const std::string &s2) const {
        return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), [](char c1, char c2) {return ::tolower(c1) < ::tolower(c2);});
    }
}; // ci

typedef std::multimap<std::string, std::string, ci> Headers;

struct MultipartFile {
    std::string filename;
    std::string content_type;
    size_t offset = 0;
    size_t length = 0;
}; // MultipartFile

typedef std::multimap<std::string, MultipartFile> MultipartFiles;

class Stream {
public:
    virtual ~Stream() {}
    virtual int read(char *ptr, size_t size) = 0;
    virtual int write(const char *ptr, size_t size1) = 0;
    virtual int write(const char *ptr) = 0;
    virtual std::string get_remote_addr() const = 0;

    template <typename... Args>
        void write_format(const char *fmt, const Args &... args);
}; // Stream

class SocketStream : public Stream {
public:
    SocketStream(socket_t sock);
    virtual ~SocketStream();

    virtual int read(char *ptr, size_t size);
    virtual int write(const char *ptr, size_t size);
    virtual int write(const char *ptr);
    virtual std::string get_remote_addr() const;

private:
    socket_t sock_;
}; // SocketStream

class BufferStream : public Stream {
public:
    BufferStream() {}
    virtual ~BufferStream() {}

    virtual int read(char *ptr, size_t size);
    virtual int write(const char *ptr, size_t size);
    virtual int write(const char *ptr);
    virtual std::string get_remote_addr() const;

    const std::string &get_buffer() const;

private:
    std::string buffer;
}; // BufferStream

struct Request {
    std::string version;
    std::string method;
    std::string target;
    std::string path;
    Headers headers;
    std::string body;
    Params params;
    MultipartFiles files;
    Match matches;

    Progress progress;

    bool has_header(const char *key) const;
    std::string get_header_value(const char *key, size_t id = 0) const;
    size_t get_header_value_count(const char *key) const;
    void set_header(const char *key, const char *val);

    bool has_param(const char *key) const;
    std::string get_param_value(const char *key, size_t id = 0) const;
    size_t get_param_value_count(const char *key) const;

    bool has_file(const char *key) const;
    MultipartFile get_file_value(const char *key) const;
}; // Request

struct Response {
    std::string version;
    int status;
    Headers headers;
    std::string body;
    std::function<std::string(uint64_t offset)> streamcb;

    bool has_header(const char *key) const;
    std::string get_header_value(const char *key, size_t id = 0) const;
    size_t get_header_value_count(const char *key) const;
    void set_header(const char *key, const char *val);

    void set_redirect(const char *uri);
    void set_content(const char *s, size_t n, const char *content_type);
    void set_content(const std::string &s, const char *content_type);

    Response() : status(-1) {}
}; // Reponse


class Server {
public:
    typedef std::function<void(const Request &, Response &)> Handler;
    typedef std::function<void(const Request &, const Response &)> Logger;

    Server();

    virtual ~Server();

    virtual bool is_valid() const;

    Server &Get(const char *pattern, Handler handler);
    Server &Post(const char *pattern, Handler handler);

    Server &Put(const char *pattern, Handler handler);
    Server &Patch(const char *pattern, Handler handler);
    Server &Delete(const char *pattern, Handler handler);
    Server &Options(const char *pattern, Handler handler);

    bool set_base_dir(const char *path);

    void set_error_handler(Handler handler);
    void set_logger(Logger logger);

    void set_keep_alive_max_count(size_t count);
    void set_payload_max_length(uint64_t length);

    int bind_to_any_port(const char *host, int socket_flags = 0);
    bool listen_after_bind();

    bool listen(const char *host, int port, int socket_flags = 0);

    bool is_running() const;
    void stop();
protected:
    bool process_request(Stream &strm, bool last_connection,
                         bool &connection_close);

    size_t keep_alive_max_count_;
    size_t payload_max_length_;

private:
    typedef std::vector<std::pair<std::regex, Handler>> Handlers;

    socket_t create_server_socket(const char *host, int port,
                                  int socket_flags) const;
    int bind_internal(const char *host, int port, int socket_flags);
    bool listen_internal();

    bool routing(Request &req, Response &res);
    bool handle_file_request(Request &req, Response &res);
    bool dispatch_request(Request &req, Response &res, Handlers &handlers);

    bool parse_request_line(const char *s, Request &req);
    void write_response(Stream &strm, bool last_connection, const Request &req,
                        Response &res);

    virtual bool read_and_close_socket(socket_t sock);

    bool is_running_;
    socket_t svr_sock_;
    std::string base_dir_;
    Handlers get_handlers_;
    Handlers post_handlers_;
    Handlers put_handlers_;
    Handlers patch_handlers_;
    Handlers delete_handlers_;
    Handlers options_handlers_;
    Handler error_handler_;
    Logger logger_;
    // TODO: Use thread pool...
    std::mutex running_threads_mutex_;
    int running_threads_;
}; // Server

#endif // INCLUDE_PUBLIC_H
