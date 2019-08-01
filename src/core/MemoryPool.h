
#ifndef INCLUDE_MEMORYPOOL_H
#define INCLUDE_MEMORYPOOL_H

#include <iostream>
#include <functional>

struct pool_data_s;
struct pool_s;
struct list_t;
struct pool_large_s;
struct pool_clean_s;
struct pool_s {
    pool_data_s* d;
    size_t max;
    pool_s* current;
    list_t* chain;
    pool_large_s* large;
    pool_clean_s* clean;
};
struct pool_data_s {
    unsigned char* last;
    unsigned char* end;
    pool_s* next;
    unsigned int failed;
};
struct pool_large_s {
    pool_large_s* next;
    void* alloc;
};

struct pool_clean_s {
    pool_clean_s* handler;
    void* data;
    pool_clean_s* next;
};

class MemoryPool {
public:
    typedef std::function<void()> Task;
    MemoryPool();
    ~MemoryPool();
private:
};

#endif // INCLUDE_MEMORYPOOL_H
