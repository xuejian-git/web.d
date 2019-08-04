/**********************************************************
 *   Author          : Apriluestc
 *   Email           : 13669186256@163.com
 *   Last modified   : 2019-08-04 11:55
 *   Filename        : fastcgi.cpp
 *   Description     : 
 * *******************************************************/

#include <iostream>
#include <cstring>
#include "FastCgi.h"

FastCgiHeader FastCgiData::MakeHeader(int type, int requestId, int contentLength, int paddingLength) {
	FastCgiHeader header;
	header.version = FASTCGI_VERSION;
	header.type             = static_cast<unsigned char>(type);
	header.requestIdB1      = static_cast<unsigned char>((requestId >> 8) & 0xff);
	header.requestIdB0      = static_cast<unsigned char>((requestId) & 0xff);
	header.contentLengthB1  = static_cast<unsigned char>((contentLength >> 8) & 0xff);
	header.contentLengthB0  = static_cast<unsigned char>((contentLength) & 0xff);
	header.paddingLength    = static_cast<unsigned char>(paddingLength);
	header.reserved         =  0;
	return header;
}

FastCgiBeginRequestBody FastCgiData::MakeBeginRequestBody(int role, int keepConn) {
	FastCgiBeginRequestBody body;
	// body.roleB1 = (unsigned char)((role >>  8) & 0xff);
	body.roleB1 = static_cast<unsigned char>((role >> 8) & 0xff);
	body.roleB0 = static_cast<unsigned char>(role & 0xff);
	body.flag  = static_cast<unsigned char>((keepConn) ? 1 : 0);
	memset(body.reserved, 0, sizeof(body.reserved));
	return body;
}

int FastCgiData::SendBeginRequestRecord(CallFunc wr, int fd, int requestId) {
    int ret;
    // 构造一个FCGI_BeginRequestRecord结构
    FastCgiBeginRequestRecord beginRecord;

    beginRecord.header = 
        MakeHeader(FASTCGI_BEGIN_REQUEST, requestId, sizeof(beginRecord.body), 0);
    beginRecord.body = MakeBeginRequestBody(FASTCGI_RESPONDER, 0);

    ret = wr(fd, &beginRecord, sizeof(beginRecord));

    if (ret == sizeof(beginRecord)) {
        return 0;
    } else {
        return -1;
    }
}

int FastCgiData::SendParamRecord(CallFunc wr, int fd, int requestId, char *name, int nlen, char *value, int vlen) {
    unsigned char *buf, *old;
    int ret, pl,  cl = nlen + vlen;
    cl = (nlen < 128) ? ++cl : cl + 4; 
    cl = (vlen < 128) ? ++cl : cl + 4; 

    // 计算填充数据长度
    pl = (cl % 8) == 0 ? 0 : 8 - (cl % 8);
    // old = buf = (unsigned char *)malloc(FASTCGI_HEADER_LENGTH + cl + pl);
    old = buf = static_cast<unsigned char*>(malloc(FASTCGI_HEADER_LENGTH + cl + pl));

    FastCgiHeader nvHeader = MakeHeader(FASTCGI_PARAMS, requestId, cl, pl);
    memcpy(buf, reinterpret_cast<char*>(&nvHeader), FASTCGI_HEADER_LENGTH);
    buf = buf + FASTCGI_HEADER_LENGTH;

    if (nlen < 128) {
        
        // name长度小于128字节，用一个字节保存长度
        *buf++ = static_cast<unsigned char>(nlen);
    } else {

        // 大于等于128用4个字节保存长度
        *buf++ = static_cast<unsigned char>((nlen >> 24) | 0x80);
        *buf++ = static_cast<unsigned char>(nlen >> 16);
        *buf++ = static_cast<unsigned char>(nlen >> 8);
        *buf++ = static_cast<unsigned char>(nlen);
    }

    if (vlen < 128) {

        // value长度小于128字节，用一个字节保存长度
        *buf++ = static_cast<unsigned char>(vlen);
    } else {
        
        // 大于等于128用4个字节保存长度
        *buf++ = static_cast<unsigned char>((vlen >> 24) | 0x80);
        *buf++ = static_cast<unsigned char>(vlen >> 16);
        *buf++ = static_cast<unsigned char>(vlen >> 8);
        *buf++ = static_cast<unsigned char>(vlen);
    }

    memcpy(buf, name, nlen);
    buf = buf + nlen;
    memcpy(buf, value, vlen);

    ret = wr(fd, old, FASTCGI_HEADER_LENGTH + cl + pl);

    free(old);

    if (ret == (FASTCGI_HEADER_LENGTH + cl + pl)) {
        return 0;
    } else {
        return -1;
    }
}

int FastCgiData::SendEmptyParamsRecord(CallFunc wr, int fd, int requestId) {
    int ret;
    FastCgiHeader nvHeader = MakeHeader(FASTCGI_PARAMS, requestId, 0, 0);
    ret = wr(fd, reinterpret_cast<char *>(&nvHeader), FASTCGI_HEADER_LENGTH);

    if (ret == FASTCGI_HEADER_LENGTH) {
        return 0;
    } else {
        return -1;
    }
}

/*
 * 发送FCGI_STDIN数据
 * 发送成功返回0
 * 出错返回-1
 */

int FastCgiData::SendStdinRecord(CallFunc wr, int fd, int requestId, char *data, int len) {
    int cl = len, pl, ret;
    char buf[8] = {0};

    while (len > 0) {
        
        // 判断STDIN数据是否大于传输最大值FCGI_MAX_LENGTH
        if (len > FASTCGI_MAX_LENGTH) {
            cl = FASTCGI_MAX_LENGTH;
        }

        // 计算填充数据长度
        pl = (cl % 8) == 0 ? 0 : 8 - (cl % 8);

        FastCgiHeader sinHeader = MakeHeader(FASTCGI_STDIN, requestId, cl, pl);

        // 发送协议头部
        ret = wr(fd, reinterpret_cast<char *>(&sinHeader), FASTCGI_HEADER_LENGTH);
        if (ret != FASTCGI_HEADER_LENGTH) {
            return -1;
        }

        // 发送 FastCgiStdin 数据
        ret = wr(fd, data, cl);
        if (ret != cl) {
            return -1;
        }

        if (pl > 0) {
            ret = wr(fd, buf, pl);
            if (ret != pl) {
                return -1;
            }
        }

        len -= cl;
        data += cl;
    }

    return 0;
}

/*
 * 发送空的FCGI_STDIN记录
 * 发送成功返回0
 * 出错返回-1
 */

int FastCgiData::SendEmptyStdinRecord(CallFunc wr, int fd, int requestId) {
    int ret;
    FastCgiHeader sinHeader = MakeHeader(FASTCGI_STDIN, requestId, 0, 0);
    ret = wr(fd, reinterpret_cast<char *>(&sinHeader), FASTCGI_HEADER_LENGTH);

    if (ret == FASTCGI_HEADER_LENGTH) {
        return 0;
    } else {
        return -1;
    }
}

/*
 * 读取php-fpm处理结果
 * 读取成功返回0
 * 出错返回-1
 */
int FastCgiData::RecvRecord(CallFunc rr, CallCli stc, int cfd, int fd, int requestId) {
    FastCgiHeader responHeader;
    FastCgiEndRequestBody endr;
    char *conBuf = NULL, *errBuf = NULL;
    int buf[8], cl, ret;
    
    // 保存fpm发送过来的request id 
    int fcgi_rid;
    int outlen = 0, errlen = 0;

    // 读取协议记录头部
    while (rr(fd, &responHeader, FASTCGI_HEADER_LENGTH) > 0) {
        fcgi_rid = static_cast<int>(responHeader.requestIdB1 << 8) + static_cast<int>(responHeader.requestIdB0);
        if (responHeader.type == FASTCGI_STDOUT && fcgi_rid == requestId) {
            
            // 获取内容长度
            cl = static_cast<int>(responHeader.contentLengthB1 << 8) + static_cast<int>(responHeader.contentLengthB0);
            
            //*outlen += cl;
            outlen += cl;

            // 如果不是第一次读取FCGI_STDOUT记录
            if (conBuf != NULL) { 
                // 扩展空间
                //conBuf = realloc(*sout, *outlen);
                // conBuf = (char*)realloc(conBuf, outlen);
                conBuf = static_cast<char*>(realloc(conBuf, outlen));
            } else {
                conBuf = static_cast<char*>(malloc(cl));
                //*sout = conBuf;
            }

            ret = rr(fd, conBuf, cl);
            if (ret == -1 || ret != cl) {
                std::cout << "read fcgi_stdout record error" << std::endl;
                return -1;
            }

            // 读取填充内容，忽略
            if (responHeader.paddingLength > 0) {
                ret = rr(fd, buf, responHeader.paddingLength);
                if (ret == -1 || ret != responHeader.paddingLength) {
                    std::cout << "read fcgi_stdout padding error" << responHeader.paddingLength << std::endl;
                    return -1;
                }
            }
        } else if (responHeader.type == FASTCGI_STDERR && fcgi_rid == requestId) {
            // 获取内容长度
            cl = static_cast<int>(responHeader.contentLengthB1 << 8) + static_cast<int>(responHeader.contentLengthB0);
            //*errlen += cl;
            errlen += cl;

            // 如果不是第一次读取FCGI_STDOUT记录
            if (errBuf != NULL) { 
                // 扩展空间
                errBuf = static_cast<char*>(realloc(errBuf, errlen));
            } else {
                // errBuf = (char *)malloc(cl);
                errBuf = static_cast<char*>(malloc(cl));
                //*serr = errBuf;
            }

            ret = rr(fd, errBuf, cl);
            if (ret == -1 || ret != cl) {
                return -1;
            }

            // 读取填充内容，忽略
            if (responHeader.paddingLength > 0) {
                ret = rr(fd, buf, responHeader.paddingLength);
                if (ret == -1 || ret != responHeader.paddingLength) {
                    return -1;
                }
            }
        } else if (responHeader.type == FASTCGI_END_REQUEST && fcgi_rid == requestId) {
            // 读取结束请求协议体
            ret = rr(fd, &endr, sizeof(FastCgiEndRequestBody));

            if (ret == -1 || ret != sizeof(FastCgiEndRequestBody)) {
                free(conBuf);
                free(errBuf);
                return -1;
            }

            stc(cfd, outlen, conBuf, errlen, errBuf, &endr);
            free(conBuf);
            free(errBuf);
            return 0;
        }
    }
    return 0;
}
