#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <logger.h>
#include "http_utils.h"
#include "socket_wrapper.h"
#include "config.h"

filetype_t type_arr[] = {
    {"*",   "application/octet-stream"}, // fallback
    {"gif", "image/gif"}, 
    {"jpg", "image/jpeg"}, 
    {"jpeg","image/jpeg"}, 
    {"png", "image/png"}, 
    {"ico", "image/x-icon"}, 
    {"htm", "text/html"}, 
    {"html","text/html"}, 
    {"mp3", "audio/mp3"}, // hey, PCM
    {"wav", "audio/wav"},
    {"ogg", "audio/ogg"},
    {"zip", "application/zip"}, 
    {"gz",  "application/x-gzip"}, 
    {"tar", "application/x-tar"}
};

statuscode_t status_arr[] = {
    {200, "OK"}, 
    {403, "Forbidden"}, 
    {404, "Not Found"}, 
    {405, "Method Not Allowed"}, 
    {501, "Not Implemented"}
};

char laji_httpd_caching_data[BUFFER_SIZE];
char laji_httpd_caching_file[161];
size_t laji_httpd_caching_size;

int laji_httpd_caching_enabled = 1;

int http_handle_read(epoll_evt_data_t* http_evt) {

    ssize_t read_size;
    char buffer[BUFFER_SIZE+1];
    int socketfd;

    socketfd = http_evt->fd;
    read_size = sfdgets(socketfd, buffer, BUFFER_SIZE);
    // should not equals to 0 since this is the only line we need.
    if (read_size <= 0) {
        perror("recv() at sfdgets() lower than 0:");
        close(socketfd);
        free(http_evt);
        return -1;
    }

    if(strncmp(buffer,"GET ",4) && strncmp(buffer,"get ",4)) {
        http_evt->type = UNSUPPORTED;
    } else {
        http_evt->type = GET;
    }

    for(int i = 4; i < read_size; i++) {
        // safe check for not allowed access out of the WWW_PATH
        if(buffer[i] == '.' && buffer[i+1] == '.') {
            // in fact it may cause problem if we use dots in get parameter, e.g. https://sample.domain/example.htm&hey=yooo..oo
            http_evt->response_code = 403;
            strcpy(buffer, "GET /index.html");
            break;
        }

        // do lazy work for default req file.
        if(buffer[i] == ' ') {
            buffer[i] = '\0';
            if (i == 5) { // lazy work: if requesting `/`, not work if requesting a sub folder.
                strcpy(buffer,"GET /index.html");
            }
            break;
        }
    }

    http_copy_urldecoded_str(http_evt->url, &buffer[5]);
    http_evt->response_code = 200;

    Epoll_ctl(http_evt->epollfd, EPOLL_CTL_ADD, http_evt->fd, EPOLLOUT, http_evt);
    
    return 0;
}

int http_handle_write(epoll_evt_data_t* http_evt) {
    
    int socketfd, filefd;
    ssize_t buffer_size;
    char buffer[BUFFER_SIZE+1], *content_type_str;

    socketfd = http_evt->fd;
    if (http_evt->type != GET) {
        http_response_error(socketfd, 405);
    }

    if (http_evt->response_code != 200) {
        http_response_error(socketfd, 403);
    }

    sprintf(buffer, "%s", http_evt->url);
    buffer_size = strlen(buffer);
    content_type_str = type_arr[0].type;

    for(int i=1; i < sizeof(type_arr); i++) {
        size_t slen = strlen(type_arr[i].ext);
        if(!strncmp(&buffer[buffer_size-slen], type_arr[i].ext, slen)) {
            content_type_str = type_arr[i].type;
            break;
        }
    }

    char* decoded_uri = http_evt->url;
    int use_caching_data = 0;

    if (laji_httpd_caching_enabled) {
        int slen = strlen(decoded_uri);
        if (strncmp(decoded_uri, laji_httpd_caching_file, slen) == 0) use_caching_data = 1;
    } 
    if (use_caching_data) {

        laji_log(LOG_VERBOSE, "Handle accept using cache.");

        write(socketfd, laji_httpd_caching_data, laji_httpd_caching_size);

    } else {

        laji_log(LOG_VERBOSE, "Handle accept normally.");

        filefd = open(decoded_uri, O_RDONLY); 
        if(filefd == -1) {
            http_response_error(socketfd, 403); return 0;// or maybe 404?
        }

        struct stat file_stat;
        fstat(filefd, &file_stat);

        sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\nConnection: close\r\nContent-Length: %ld\r\n\r\n", content_type_str, file_stat.st_size);
        write(socketfd,buffer,strlen(buffer));

        if (laji_httpd_caching_enabled && file_stat.st_size < 4096) {
            laji_httpd_caching_size = 0;
            while ((buffer_size = read(filefd, buffer, BUFFER_SIZE)) > 0 ) {
                memcpy(laji_httpd_caching_data + laji_httpd_caching_size, buffer, buffer_size);
                laji_httpd_caching_size += buffer_size;
                write(socketfd, buffer, buffer_size);
            }
        } else {
            while ((buffer_size = read(filefd, buffer, BUFFER_SIZE)) > 0 ) {
                write(socketfd, buffer, buffer_size);
            }
        }
        

        close(filefd);
    }

    read(socketfd, buffer, sizeof(buffer)); // anyone tell me why need read() here?
    close(http_evt->fd);

    return 0;
}

int http_response_error(int socketfd, int status_code) {

    char buffer[BUFFER_SIZE+1], *status_str;
    status_str = status_arr[0].desc;
    for(int i=1; i < sizeof(status_arr); i++) {
        if (status_arr[i].code == status_code) status_str = status_arr[i].desc;
    }

    sprintf(buffer,"HTTP/1.0 %d %s\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: 4\r\n\r\n%d\n", status_code, status_str, status_code);
    write(socketfd,buffer,strlen(buffer));

    return 0;

}

int http_caching_toggle(int caching_enabled) {
    laji_httpd_caching_enabled = caching_enabled == 0 ? 0 : 1;
}

int http_copy_urldecoded_str(char* dest, char *pstr) {

    char *buf = (char*)malloc(strlen(pstr) + 1), *pbuf = buf;
    while (*pstr) {
        if (*pstr == '%') {
            if (pstr[1] && pstr[2]) {
                *pbuf++ = hex2char(pstr[1]) << 4 | hex2char(pstr[2]);
                pstr += 2;
            }
        } else if (*pstr == '+') { 
            *pbuf++ = ' ';
        } else {
            *pbuf++ = *pstr;
        }
        pstr++;
    }
    *pbuf = '\0';

    strcpy(dest, buf);
    free(buf);

    return 0;
}

char hex2char(char ch) {
    return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}
