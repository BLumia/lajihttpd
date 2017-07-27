#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <logger.h>
#include "socket_wrapper.h"
#include "epoll_manager.h"
#include "http_utils.h"

int epollworkerfds[EPOLL_WORKER_COUNT];
int epolldispatcherfd;
volatile int epoll_do_shutdown = 0;
struct epoll_event active_evt_pool[MAXEVENTS];

int epollworker_main();
int epollmgr_dispatch_acceptfd(int acceptfd);
int epolldispatcher_main(int listenfd);
int epollmgr_init(int listenfd);

int epollworker_main() {

    for (int i = 0; i < EPOLL_WORKER_COUNT; i++) {
        epollworkerfds[i] = epoll_create1(0);
        if (epollworkerfds[i] == -1) {
            perror("epoll_create1() at epollworker_main()");
            laji_log(LOG_ERROR, "epoll_create1() at epollworker_main()");
            return -1;
        }
    }
    return 0;
}

int epollworker_eventloop(int epollfd) {

    int connfd;
    struct epoll_event worker_evt_pool[MAXEVENTS];

    for(;;) {
        int evt_cnt = epoll_wait(epollfd, worker_evt_pool, MAXEVENTS, -1);
        for (int i = 0; i < evt_cnt; i++) {

            int events = worker_evt_pool[i].events;
            epoll_evt_data_t* http_evt = (epoll_evt_data_t*)worker_evt_pool[i].data.ptr;

            if(events & EPOLLHUP || events & EPOLLERR) { 
                perror("epoll main loop (not listenfd), EPOLLHUP or EPOLLERR");
                close(http_evt->fd);
                free(http_evt);
            } else if (EPOLLIN == events) { // r
                http_evt->event = EPOLLIN;
                http_evt->epollfd = epollfd;
                Epoll_ctl(http_evt->epollfd, EPOLL_CTL_DEL, http_evt->fd, 0, 0);
                http_handle_read(http_evt);
            } else if (EPOLLOUT == events) { // w
                http_evt->event = EPOLLOUT;
                http_evt->epollfd = epollfd;
                Epoll_ctl(http_evt->epollfd, EPOLL_CTL_DEL, http_evt->fd, 0, 0);
                http_handle_write(http_evt); // free `http_evt` inside http_handle_write()
            }
        }
        if (epoll_do_shutdown) break;
    }
}

// this will block the thread. Ensure the workers are running first.
int epolldispatcher_main(int listenfd) {

    int connfd, ret;

    epolldispatcherfd = epoll_create1(0);
    if (epolldispatcherfd < 0) {
        perror("epoll_create1() at epolldispatcher_main()");
        laji_log(LOG_ERROR, "epoll_create1() at epolldispatcher_main()");
    }

    ret = Epoll_ctl(epolldispatcherfd, EPOLL_CTL_ADD, listenfd, EPOLLIN, &listenfd);
    if (ret < 0) exit(EXIT_FAILURE);

    for(;;) {
        int evt_cnt = epoll_wait(epolldispatcherfd, active_evt_pool, MAXEVENTS, -1);
        for (int i = 0; i < evt_cnt; i++) {

            int events = active_evt_pool[i].events;
            epoll_evt_data_t* http_evt = (epoll_evt_data_t*)active_evt_pool[i].data.ptr;

            if ((int*)http_evt == &listenfd) { // accepting reqs.
                if (events & EPOLLHUP || events & EPOLLERR) { // err
                    close(listenfd); // no need for free http_evt because it's &acceptfd
                    exit(EXIT_FAILURE);
                } else { // do accepting (and dispatch?)
                    struct sockaddr in_addr;
                    socklen_t in_len = sizeof(in_addr);
                    connfd = Accept(listenfd, (SA*) &in_addr, &in_len);
                    set_nonblocking(connfd);
                    epollmgr_dispatch_acceptfd(connfd);
                }
            } else { // the accepted events, maybe useless when dispatcher got implemented.
                if(events & EPOLLHUP || events & EPOLLERR) { 
                    perror("epoll main loop (not listenfd), EPOLLHUP or EPOLLERR");
                    close(http_evt->fd);
                    free(http_evt);
                } else if (EPOLLIN == events) { // r
                    http_evt->event = EPOLLIN;
                    http_evt->epollfd = epolldispatcherfd;
                    Epoll_ctl(http_evt->epollfd, EPOLL_CTL_DEL, http_evt->fd, 0, 0);
                    http_handle_read(http_evt);
                } else if (EPOLLOUT == events) { // w
                    http_evt->event = EPOLLOUT;
                    http_evt->epollfd = epolldispatcherfd;
                    Epoll_ctl(http_evt->epollfd, EPOLL_CTL_DEL, http_evt->fd, 0, 0);
                    http_handle_write(http_evt);
                }
            }
        }
    }
}

int epollmgr_dispatch_acceptfd(int acceptfd) {
    // currently we do not dispatch, just the same epoll
    epoll_evt_data_t* http_evt = malloc(sizeof(epoll_evt_data_t));
    http_evt->fd = acceptfd;
    Epoll_ctl(epolldispatcherfd, EPOLL_CTL_ADD, acceptfd, EPOLLIN, http_evt); // maybe plus EPOLLONESHOT ?
}

int epollmgr_init(int listenfd) {

    epolldispatcher_main(listenfd);


}

int epollmgr_shutdown() {

    epoll_do_shutdown = 1;

    // join threads?


}
