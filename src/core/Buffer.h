/**********************************************************
 *   Author          : Apriluestc
 *   Email           : 13669186256@163.com
 *   Last modified   : 2019-07-28 20:20
 *   Filename        : Buffer.h
 *   Description     : 
 *   TCP 接收发送环形缓冲区
 * *******************************************************/

#ifndef INCLUDE_BUFFER_H
#define INCLUDE_BUFFER_H

#define LARGE_PAGE_NODE 12

struct BufferNode {
    char* msg;
    int size;
    BufferNode* next;
    char* buf;
    int capacity;
}; // BufferNode

struct BufferPool {
    BufferNode* head;
    int len;
}; // BufferPool

struct SocketBuffer {
    int size;
    int offset;
    BufferNode* head;
    BufferNode* tail;
    BufferPool* pool;
}; // SocketBuffer

class Buffer {
public:
    Buffer();
    ~Buffer();
    SocketBuffer* SocketBufferNew();
    void SocketBufferFree(SocketBuffer* sp);
    int BufferPushData(SocketBuffer* sp, char* msg, int size);
    void BufferNodeRelease(SocketBuffer* sp);
    char* BufferReadSpec(SocketBuffer* sp, int size, int* realSz);
    char* BufferReadAll(SocketBuffer* sp, int* retNum);
    int BufferGetSize(SocketBuffer* sp);
    BufferNode* NewBufferNode(int size);
}; // Buffer

#endif // INCLUDE_BUFFER_H
