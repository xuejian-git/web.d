#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

#define HTTP_INVALID_METHOD  1
#define HTTP_INVALID_REQUEST 2
#define HTTP_INVALID_HEAD    3

#define HTTP_UNKNOWN 1
#define HTTP_GET     2
#define HTTP_HEAD    3
#define HTTP_POST    4

#define CR '\r'
#define LF '\n'

//请求头结构
typedef struct http_request_header_s{
    void *key;
    void *key_end;
    void *value;
    void *value_end;
    struct http_request_header_s *next;
} http_request_header_t;

//请求结构
typedef struct {
    int fd;
    char buffer[BUFFER_SIZE];
    size_t check_index;
    size_t read_index;
    int state;

    //请求行
    void *request_start;
    void *request_end;
    int method;
    void *method_start;
    void *method_end;
    void *uri;
    void *uri_end;
    int major_digit;
    int minor_digit;

    //请求头部
    http_request_header_t *header_list; //头部字段链表
    void *cur_key;
    void *cur_key_end;
    void *cur_value;
    void *cur_value_end;
} http_request_t;

//向请求头链表添加新结点
void http_header_list_add(http_request_header_t **head, http_request_header_t *p) {
    if (*head == NULL) {
        *head = p;
        p->next = NULL;
        return;
    }
    http_request_header_t *t = (*head)->next;
    (*head)->next = p;
    p->next = t;
}

//解析请求行
int http_parse_request_line(http_request_t *request) {
    enum {
        sw_start = 0,
        sw_method,
        sw_spaces_before_uri,
        sw_after_slash_in_uri,
        sw_http,
        sw_http_H,
        sw_http_HT,
        sw_http_HTT,
        sw_http_HTTP,
        sw_first_major_digit,
        sw_major_digit,
        sw_first_minor_digit,
        sw_minor_digit,
        sw_spaces_after_digit,
        sw_almost_done
    } state;

    state = request->state;

    char ch, *p;
    size_t pi;
    for (pi = request->check_index; pi < request->read_index; pi++){
        p = &request->buffer[pi];
        ch = *p;
        switch (state) {
        case sw_start:
            {
                request->request_start = p;
                if (ch == CR || ch == LF)
                    break;
                if (ch == ' ')
                    break;
                if (ch < 'A' || ch > 'Z')
                    return HTTP_INVALID_REQUEST;

                request->method_start = p;
                state = sw_method;
                break;
            }

        case sw_method:
            {
                if (ch == ' '){
                    char* m = static_cast<char*>(request->method_start);
                    if (p-m == 3) {
                        if (m[0] == 'G' && m[1] == 'E' && m[2] == 'T') {
                            request->method_end = p;
                            request->method = HTTP_GET;
                        }
                    }
                    else if (p-m == 4) {
                        if (m[0] == 'H' && m[1] == 'E' && m[2] == 'A' && m[3] == 'D') {
                            request->method_end = p;
                            request->method = HTTP_HEAD;
                        }
                        else if (m[0] == 'P' && m[1] == 'O' && m[2] == 'S' && m[3] == 'T') {
                            request->method_end = p;
                            request->method = HTTP_POST;
                        }
                    }
                    else {
                        request->method_end = p;
                        request->method = HTTP_UNKNOWN;
                    }
                    state = sw_spaces_before_uri;
                    break;
                }
                if (ch < 'A' || ch > 'Z')
                    return HTTP_INVALID_METHOD;
                break;
            }

        case sw_spaces_before_uri:
            {
                if (ch == '/') {
                    request->uri = p+1;
                    state = sw_after_slash_in_uri;
                    break;
                }

                if (ch == ' ')
                    break;
                else
                    return HTTP_INVALID_REQUEST;
                break;
            }

        case sw_after_slash_in_uri:
            {
                if (ch == ' ') {
                    request->uri_end = p;
                    state = sw_http;
                    break;
                }
                break;
            }

        case sw_http:
            {
                if (ch == ' ')
                    break;
                if (ch == 'H') {
                    state = sw_http_H;
                    break;
                }
                return HTTP_INVALID_REQUEST;
                break;
            }

        case sw_http_H:
            {
                if (ch == 'T') {
                    state = sw_http_HT;
                    break;
                }       
                return HTTP_INVALID_REQUEST;
                break;
            }

        case sw_http_HT:
            {
                if (ch == 'T') {
                    state = sw_http_HTT;
                    break;
                }       
                return HTTP_INVALID_REQUEST;
                break;
            }

        case sw_http_HTT:
            {   
                if (ch == 'P') {
                    state = sw_http_HTTP;
                    break;
                }       
                return HTTP_INVALID_REQUEST;
                break;
            }

        case sw_http_HTTP:
            {
                if (ch == '/') {
                    state = sw_first_major_digit;
                    break;
                }       
                return HTTP_INVALID_REQUEST;
                break;
            }

        case sw_first_major_digit:
            {
                if (ch >= '0' && ch <= '9') {
                    request->major_digit = ch - '0';
                    state = sw_major_digit;
                    break;
                }
                return HTTP_INVALID_REQUEST;
                break;
            }

        case sw_major_digit:
            {
                if (ch == '.') {
                    state = sw_first_minor_digit;
                    break;
                }       
                if (ch >= '0' && ch <= '9') {
                    request->major_digit = request->major_digit * 10 + ch - '0';
                    break;
                }
                return HTTP_INVALID_REQUEST;
                break;
            }

        case sw_first_minor_digit:
            {
                if (ch >= '0' && ch <= '9') {
                    request->minor_digit = ch - '0';
                    state = sw_minor_digit;
                    break;
                }
                return HTTP_INVALID_REQUEST;
                break;
            }

        case sw_minor_digit:
            {
                if (ch == ' ') {
                    state = sw_spaces_after_digit;
                    break;
                }
                if (ch == CR) {
                    state = sw_almost_done;
                    break;
                }
                if (ch == LF)
                    goto done;
                if (ch >= '0' && ch <= '9') {
                    request->minor_digit = request->minor_digit * 10 + ch - '0';
                    break;
                }
                return HTTP_INVALID_REQUEST;
                break;
            }

        case sw_spaces_after_digit:
            {
                if (ch == ' ')
                    break;
                if (ch == CR) {
                    state = sw_almost_done;
                    break;
                }
                if (ch == LF) {
                    goto done;
                    break;
                }
                return HTTP_INVALID_REQUEST;
                break;
            }

        case sw_almost_done:
            {
                request->request_end = p-1;
                if (ch == LF)
                    goto done;
                return HTTP_INVALID_REQUEST;
                break;
            }
        }
    }

done:
    request->check_index = pi + 1;
    if (request->request_end == NULL)
        request->request_end = p;
    request->state = sw_start;
    return 0;
}

//解析请求头部
int http_parse_request_header(http_request_t *request) {
    enum {
        sw_start = 0,
        sw_key,
        sw_spaces_before_colon,
        sw_spaces_after_colon,
        sw_value,
        sw_cr,
        sw_crlf,
        sw_crlfcr
    } state;

    state = request->state;

    char ch, *p;
    size_t pi;
    http_request_header_t *hd;

    for (pi = request->check_index; pi < request->read_index; pi++) {
        p = &request->buffer[pi];
        ch = *p;

        switch (state) {
        case sw_start:
            {
                if (ch == CR || ch == LF)
                    break;
                request->cur_key = p;
                state = sw_key;
                break;
            }

        case sw_key:
            {
                if (ch == ' ') {
                    request->cur_key_end = p;
                    state = sw_spaces_before_colon;
                    break;
                }
                if (ch == ':') {
                    request->cur_key_end = p;
                    state = sw_spaces_after_colon;
                    break;
                }
                break;
            }

        case sw_spaces_before_colon:
            {
                if(ch == ' ')
                    break;
                if (ch == ':') {
                    state = sw_spaces_after_colon;
                    break;
                }
                return  HTTP_INVALID_HEAD;
                break;
            }

        case sw_spaces_after_colon:
            {
                if (ch == ' ')
                    break;
                request->cur_value = p;
                state = sw_value;
                break;
            }

        case sw_value:
            {
                if (ch == CR) {
                    request->cur_value_end = p;
                    state = sw_cr;
                    break;
                }
                if (ch == LF) {
                    request->cur_value_end = p;
                    state = sw_crlf;
                    break;
                }
                break;
            }

        case sw_cr:
            {
                if (ch == LF) {
                    state = sw_crlf;
                    hd = (http_request_header_t*)malloc(sizeof(http_request_header_t));
                    hd->key = request->cur_key;
                    hd->key_end = request->cur_key_end;
                    hd->value = request->cur_value;
                    hd->value_end = request->cur_value_end;
                    http_header_list_add(&request->header_list, hd);
                    hd = NULL;
                    break;
                }         
                return HTTP_INVALID_HEAD;
                break;
            }

        case sw_crlf:
            {
                if (ch == CR) {
                    state = sw_crlfcr;
                    break;
                } else if (ch == LF) {
                    hd = (http_request_header_t*)malloc(sizeof(struct http_request_header_s));
                    hd->key = request->cur_key;
                    hd->key_end = request->cur_key_end;
                    hd->value = request->cur_value;
                    hd->value_end = request->cur_value_end;
                    http_header_list_add(&request->header_list, hd);
                    hd = NULL;
                    goto done;
                } else {
                    request->cur_key = p;
                    state = sw_key;
                }
                break;
            }

        case sw_crlfcr:
            {
                if (ch == LF)
                    goto done;
                return HTTP_INVALID_HEAD;
                break;
            }
        }
    }

done:
    request->check_index = pi + 1;
    state = sw_start;
    return 0;
}

//初始化请求结构
void http_request_init(http_request_t* q, int fd) {
    q->fd = fd;
    q->check_index = q->read_index = 0;
    q->state = 0;

    q->request_start = NULL;
    q->method = 0;
    q->method_end = NULL;
    q->uri = q->uri_end = NULL;
    q->major_digit = q->minor_digit = 0;

    q->header_list = NULL;
    q->cur_key = q->cur_key_end = NULL;
    q->cur_value = q->cur_value_end = NULL;
}

//测试
int main(int argc, char* argv[])
{
    if (argc != 2)
        printf("syntex error!\n");

    char* filename = argv[1];
    int fd = open(filename, O_RDONLY);
    int rd;
    http_request_t *request = (http_request_t*)malloc(sizeof(http_request_t));

    http_request_init(request, fd);

    rd = read(request->fd, request->buffer, BUFFER_SIZE);
    request->read_index += rd;

    if (rd <= 0) {
        printf("read error!\n");
        return 1;
    }

    if (http_parse_request_line(request) != 0) {
        printf("line parse error!\n");
        return 1;
    }

    if (http_parse_request_header(request) != 0) {
        printf("header parse error!\n");
        return 1;
    }

    printf("request:\n%s", request->buffer);
    printf("method = %d\n", request->method);
    printf("uri = ");
    for (void *p = request->uri; p != request->uri_end; p++)
        printf("%c", *(char*)p);
    printf("\n");
    printf("version: %d.%d\n", request->major_digit, request->minor_digit);
    printf("headers:\n");
    for (http_request_header_t* h = request->header_list; h != NULL; h = h->next ){
        for (void *p = h->key; p != h->key_end; p++)
            printf("%c", *(char*)p);
        printf(" : ");
        for (void *p = h->value; p != h->value_end; p++)
            printf("%c", *(char*)p);
        printf("\n");
    }
}
