#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "socket_wrapper.h"

int Socket(int family, int type, int protocol) {
    int n;
    if((n = socket(family, type, protocol)) < 0) {
	    perror("socket()");
	    return -1;
    }
    return n;
}

int Epoll_ctl(int epollfd, int op, int sfd, uint32_t evt, void* data) {
    struct epoll_event event;
    event.events = evt;
    event.data.ptr = data;
    if (epoll_ctl(epollfd, op, sfd, &event) == -1) {
        perror("epoll_ctl()");
        return -1;
    }
    return 0;
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr) {
    int n;
AGAIN:
    if ( (n = accept(fd, sa, salenptr)) < 0) {
#ifdef EPROTO
        if (errno == EPROTO || errno == ECONNABORTED)
#else
        if (errno == ECONNABORTED)
#endif
            goto AGAIN;
        else {
            perror("accept()");
            return -1;
        }
    }
    return n;
}

int create_and_bind(int port) {
    static struct sockaddr_in srv_addr; 
    int listenfd;
    // open socket and start listening
	if ((listenfd = Socket(AF_INET, SOCK_STREAM,0)) < 0) {
		exit(EXIT_FAILURE);
	}

    int reuse_addr = 1;
    if((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)))<0) {  
        perror("setsockopt()");  
        exit(EXIT_FAILURE);  
    } 
	
	srv_addr.sin_family = AF_INET; // IPv4 Socket
	srv_addr.sin_port = htons(port); 
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // should Listening at 0.0.0.0 (INADDR_ANY)
	
	if (bind(listenfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) <0) {
		perror("bind() at create_and_bind()");
		exit(EXIT_FAILURE);
	} 
    return listenfd;
}

int set_nonblocking(int fd) {  
    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1)  
        flags = 0;  
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);  
}

ssize_t sfdgets(int socketfd, char* buffer, int size) {
    
    ssize_t received, i = 0;
    char ch = '\0';
    
    while(i < (size - 1)) {
        if (ch == '\n') break;
        received = recv(socketfd, &ch, 1, 0);
        if (received > 0) {
            if (ch == '\r') { // CR LF -> LF, CR -> LF
                received = recv(socketfd, &ch, 1, MSG_PEEK);
                if (received > 0 && ch == '\n') recv(socketfd, &ch, 1, 0);
                else ch = '\n';
            }
            buffer[i] = ch;
            i++;
        } else {
            break;
        }
    }
    
    buffer[i] = '\0';
    return i;
}