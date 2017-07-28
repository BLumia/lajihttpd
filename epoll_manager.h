#ifndef __LAJIHTTPD_EPOLL_MGR
#define __LAJIHTTPD_EPOLL_MGR

#define EPOLL_WORKER_COUNT 3
#define MAXEVENTS 1024

int epollmgr_init(int listenfd);
int epollmgr_shutdown();

#endif /* __LAJIHTTPD_EPOLL_MGR */