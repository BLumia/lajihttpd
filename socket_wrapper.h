#ifndef __LAJIHTTPD_SKT_WPR
#define __LAJIHTTPD_SKT_WPR

#include <stdio.h>
#include <stdint.h>

typedef struct sockaddr SA;

// UNP style wrapper
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
int Socket(int family, int type, int protocol);
int Epoll_ctl(int epollfd, int op, int sfd, uint32_t evt, void* data);

// utils
ssize_t sfdgets(int socketfd, char* buffer, int size);
int create_and_bind(int port);
int set_nonblocking(int fd);

#endif /* __LAJIHTTPD_SKT_WPR */