/**********************************************************
 *   Author          : Apriluestc
 *   Email           : 13669186256@163.com
 *   Last modified   : 2019-08-04 13:10
 *   Filename        : Buffer.cpp
 *   Description     : 
 * *******************************************************/

#include <stdlib.h>
#include <string.h>

#include "Buffer.h"

SocketBuffer* Buffer::SocketBufferNew() {
    SocketBuffer* sb = reinterpret_cast<SocketBuffer*>(malloc(sizeof(SocketBuffer)));
    sb->size = 0;
    sb->offset = 0;
    sb->head = sb->tail = nullptr;
    sb->pool = reinterpret_cast<BufferPool*>(malloc(sizeof(BufferPool)));
    sb->pool->head = nullptr;
    sb->pool->len = 0;
    return sb;
}

void Buffer::SocketBufferFree(SocketBuffer* sb) {
    BufferNode* head = sb->pool->head;
    for (; head != nullptr; head = head->next) {
        if (head->msg) {
            free(head->msg);
            head->msg = nullptr;
        }
    }
}

int Buffer::BufferPushData(SocketBuffer* sb, char* msg, int size) {
    if (msg == nullptr || size == 0) {
        return 0;
    }
    BufferPool* pool = sb->pool;
    BufferNode* free_node = nullptr;
    if (pool->head == nullptr) {
        int len = pool->len + 1;
        int size = 8;
        if (len <= LARGE_PAGE_NODE - 3 ) {
            size -= len;
        } else {
            size -= LARGE_PAGE_NODE - 3;
        }
        free_node = NewBufferNode(size);
        pool->len = size;
    } else {
        free_node = pool->head;
    }   
    pool->head = free_node->next;
    char* msgt = static_cast<char*>(malloc(size));
    memcpy(msgt, msg, size);
    free_node->msg = msgt;
    free_node->size = size;
    free_node->next = nullptr;
    if (sb->head == nullptr)  {
        sb->head = sb->tail = free_node;
    }  else  {
        sb->tail->next = free_node;
        sb->tail = free_node;
    }
    sb->size += size;
    return sb->size;
}

void Buffer::BufferNodeRelease(SocketBuffer* sb) {
    BufferPool* pool = sb->pool;
    BufferNode* free_node = sb->head;
    sb->offset = 0;
    sb->head = free_node->next;
    if (sb->head == nullptr) {
        sb->tail = nullptr;
    }
    free_node->next = pool->head;
    free(free_node->msg);
    free_node->msg = nullptr;
    free_node->size = 0;
    pool->head = free_node;
}

char* Buffer::BufferReadSpec(SocketBuffer* sb, int size, int* realsize) {
    if (size == 0)  {
        return nullptr;
    }
    if (sb->size < size) {
        if (realsize) {
            return BufferReadAll(sb, realsize);
        }
        else {
            return nullptr;
        }
    }
    // BufferPool* pool = sb->pool;   
    sb->size -= size;
    BufferNode* cur = sb->head;
    char* msg = static_cast<char*>(malloc(size));
    int curLen = cur->size - sb->offset;
    if (size <= curLen) {
        memcpy(msg, cur->msg + sb->offset, size);
        sb->offset += size;
        if (size == curLen) {
            BufferNodeRelease(sb);    
        }
        return msg;
    }
    else {
        int offset = 0;
        for (;;) {
            int curLen = cur->size - sb->offset;
            if (curLen >= size) {
                memcpy(msg + offset, cur->msg + sb->offset, size);
                offset += size;
                sb->offset += size;
                if (curLen == size) {
                    BufferNodeRelease(sb);
                }
                break;
            }
            int real_size = (size < curLen) ? size : curLen;
            memcpy(msg + offset, cur->msg + sb->offset, real_size);
            offset += real_size;
            BufferNodeRelease(sb);
            size -= curLen;
            if (size == 0) {
                break;
            }
            cur = sb->head;
            if (!cur)   { 
                break;
            }
        }
        return msg;
    }
    return nullptr;
}

char* Buffer::BufferReadAll(SocketBuffer* sb, int* retNum) {
    int total_size = sb->size;
    if (total_size <= 0) {
        return nullptr;
    }
    char* msg = static_cast<char*>(malloc(total_size + 1));
    int offset = 0;
    while (sb->head) {
        BufferNode* cur = sb->head;
        int curLen = cur->size - sb->offset;
        memcpy(msg + offset, cur->msg + sb->offset, curLen);
        offset += curLen; 
        BufferNodeRelease(sb);
    }
    if (retNum) {
        *retNum = offset;
    }
    sb->size = 0;
    return msg;
}

int Buffer::BufferGetSize(SocketBuffer* sb) {
    if (sb)
        return sb->size;
    return 0;
}

BufferNode* Buffer::NewBufferNode(int size) {
    BufferNode** pool =  reinterpret_cast<BufferNode**>(malloc(sizeof(BufferNode*) * size));
    int i = 0;
    for (i = 0; i < size; i++)  {
        pool[i] = reinterpret_cast<BufferNode*>(malloc(sizeof(BufferNode)));
    }
    for (i = 0; i < size; i++)  {
        pool[i]->msg = nullptr;
        pool[i]->size = 0;
        pool[i]->next = pool[i + 1];
    }
    pool[size - 1]->next = 0;
    return pool[0];
}
