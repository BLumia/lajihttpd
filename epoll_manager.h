#ifndef __LAJIHTTPD_EPOLL_MGR
#define __LAJIHTTPD_EPOLL_MGR

#define EPOLL_COUNT 3
#define MAXEVENTS 64

int epollmgr_init(int listenfd);

#endif /* __LAJIHTTPD_EPOLL_MGR */