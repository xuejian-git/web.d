/**********************************************************
 *   Author          : Apriluestc
 *   Email           : 13669186256@163.com
 *   Last modified   : 2019-07-28 13:34
 *   Filename        : FileUtil.h
 *   Description     : 
 * *******************************************************/

#ifndef INCLUDE_FILEUTIL_H
#define INCLUDE_FILEUTIL_H

#include <boost/noncopyable.hpp>
#include <string>

class AppendFile : boost::noncopyable {
public:
    explicit AppendFile(std::string filename);
    ~AppendFile();
    // append 会向文件写
    void append(const char *logline, const size_t len);
    void flush();

private:
    size_t write(const char *logline, size_t len);
    FILE* fp_;
    char buffer_[64*1024];
}; // AppendFile

#endif // INCLUDE_FILEUTIL_H
