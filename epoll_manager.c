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
pthread_t epollworkerthreads[EPOLL_WORKER_COUNT];
int epolldispatcherfd;
volatile int epoll_do_shutdown = 0;
struct epoll_event active_evt_pool[MAXEVENTS];

int epollworker_main();
int epollmgr_dispatch_acceptfd(int acceptfd);
int epolldispatcher_main(int listenfd);
int epollmgr_init(int listenfd);
void* epollworker_eventloop(void *arg);

int epollworker_main() {

    laji_log(LOG_INFO, "Setting up %d epoll workers...", EPOLL_WORKER_COUNT);
    
    for (int i = 0; i < EPOLL_WORKER_COUNT; i++) {
        epollworkerfds[i] = epoll_create1(0);
        if (epollworkerfds[i] == -1) {
            perror("epoll_create1() at epollworker_main()");
            laji_log(LOG_ERROR, "epoll_create1() at epollworker_main()");
            return -1;
        }
    }

    // run event loop for workers
    for (int i = 0; i < EPOLL_WORKER_COUNT; i++) {
        int* new_i = malloc(sizeof(int));
        *new_i = i;
        pthread_create(&epollworkerthreads[i], NULL, epollworker_eventloop, (void*)new_i);
    }
    return 0;
}

void* epollworker_eventloop(void *arg) {

    int connfd, epollidx, epollfd;
    struct epoll_event worker_evt_pool[MAXEVENTS];

    epollidx = *(int*)arg;
    epollfd = epollworkerfds[epollidx];

    laji_log(LOG_INFO, "Epoll worker %d ready!", epollidx + 1);

    for(;;) {
        int evt_cnt = epoll_wait(epollfd, worker_evt_pool, MAXEVENTS, -1);
        for (int i = 0; i < evt_cnt; i++) {

            int events = worker_evt_pool[i].events;
            epoll_evt_data_t* http_evt = (epoll_evt_data_t*)worker_evt_pool[i].data.ptr;

            if(events & EPOLLHUP || events & EPOLLERR) { 
                perror("epoll main loop (not listenfd), EPOLLHUP or EPOLLERR");
                http_handle_close(http_evt);
            } else if (EPOLLIN & events) { // r
                http_evt->event = EPOLLIN;
                http_evt->epollfd = epollfd;
                http_handle_read(http_evt);
            } else if (EPOLLOUT & events) { // w
                http_evt->event = EPOLLOUT;
                http_evt->epollfd = epollfd;
                http_handle_write(http_evt); // free `http_evt` inside http_handle_write()
            }
        }
        if (epoll_do_shutdown) break; // this will not works, since it will block at `epoll_wait`
    }

    free(arg);

    laji_log(LOG_INFO, "Epoll worker stopped!", epollidx + 1);
    // should we close epollfd ?

    return NULL;
}

// this will block the thread. Ensure the workers are running first.
int epolldispatcher_main(int listenfd) {

    laji_log(LOG_INFO, "Setting up epoll event dispatcher...");

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
            } else { 
                // dispatcher dispatched the acceptfds to other epoll workers
                // so there should NOT have any other fds.
                // remove the `if` statement if my theory is right
                laji_log(LOG_ERROR, "Not halal socketfd in main dispatcher epoll.");
            }
        }
        if (epoll_do_shutdown) break; // this will not works, since it will block at `epoll_wait`
    }
}

int epollmgr_dispatch_acceptfd(int acceptfd) {
    // currently we do not dispatch, just the same epoll
    epoll_evt_data_t* http_evt = malloc(sizeof(epoll_evt_data_t));
    http_evt->fd = acceptfd;
    // since it is NOT neccessary to make workers load too balance
    // we use simple %%%%%%%%%%%%%%%% :P
    int epollfd = epollworkerfds[acceptfd % EPOLL_WORKER_COUNT];
    Epoll_ctl(epollfd, EPOLL_CTL_ADD, acceptfd, EPOLLIN /*| EPOLLONESHOT*/, http_evt); 

    return 0;
}

int epollmgr_init(int listenfd) {

    epollworker_main();
    epolldispatcher_main(listenfd);

}

int epollmgr_shutdown() {

    epoll_do_shutdown = 1;

    // join threads?
    //for (int i = 0; i < EPOLL_WORKER_COUNT; i++) {
    //    pthread_join(epollworkerthreads[i], NULL);
    //}

    return 0;
}
