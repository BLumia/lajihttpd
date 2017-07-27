#ifndef __LAJIHTTPD_HTTP_UTILS
#define __LAJIHTTPD_HTTP_UTILS

typedef enum request_type {
    UNSUPPORTED, GET, POST, PUT, DELETE, EPOLLFD /*, BLAHBLAH */
} request_type_t; // we only support GET.

typedef struct supported_filetype {
	char* ext;
	char* type;
} filetype_t;

typedef struct http_status_code {
	int code;
	char* desc;
} statuscode_t;

typedef struct epoll_eventdata {
    int fd;
    int epollfd;
    int event;
    request_type_t type;
    char url[161];
    int response_code;
} epoll_evt_data_t;

int http_handle_read(epoll_evt_data_t* http_evt);
int http_handle_write(epoll_evt_data_t* http_evt);
int http_copy_urldecoded_str(char* dest, char *pstr);
int http_caching_toggle(int caching_enabled);
int http_keepalive_toggle(int keepalive_enabled);
int http_response_error(epoll_evt_data_t* http_evt, int status_code);
char hex2char(char ch);

#endif /* __LAJIHTTPD_HTTP_UTILS */