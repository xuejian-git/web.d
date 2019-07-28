/**********************************************************
 *   Author          : Apriluestc
 *   Email           : 13669186256@163.com
 *   Last modified   : 2019-07-28 13:31
 *   Filename        : Logging.h
 *   Description     : 
 * *******************************************************/

#ifndef INCLUDE_LOGGING_H
#define INCLUDE_LOGGING_H

#include <pthread.h>
#include <string.h>
#include <string>
#include <stdio.h>

#include "LogStream.h"

class AsyncLogging;


class Logger {
public:
    Logger(const char *fileName, int line);
    ~Logger();
    LogStream& stream() { return impl_.stream_; }

    static void setLogFileName(std::string fileName) {
        logFileName_ = fileName;
    }
    static std::string getLogFileName() {
        return logFileName_;
    }

private:
    class Impl {
    public:
        Impl(const char *fileName, int line);
        void formatTime();

        LogStream stream_;
        int line_;
        std::string basename_;
    };
    Impl impl_;
    static std::string logFileName_;
}; // Logger

#define LOG Logger(__FILE__, __LINE__).stream()

#endif // INCLUDE_LOGGING_H
