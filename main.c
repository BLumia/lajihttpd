#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <configman.h>
#include <logger.h>
#include "config.h"
#include "socket_wrapper.h"
#include "epoll_manager.h"

httpd_config_t httpd_cfg;
int listenfd;

int httpd_setup_main();
int httpd_shutdown_main();

int main(int argc, char** argv) {

    httpd_setup_main();

    // ???????????

    httpd_shutdown_main();

}

int httpd_setup_main() {

    laji_log_init("./log/");
    httpd_config_main(&httpd_cfg);

    if (chdir(httpd_cfg.WWW_PATH) == -1) { 
        printf("Error: Can not change to directory '%s'\n", httpd_cfg.WWW_PATH);
        perror("Reason");
        exit(EXIT_FAILURE);
    }
    if (httpd_cfg.PORT < 0 || httpd_cfg.PORT > 65535) {
        printf("Error: Invalid port number '%d', should be in (0~65535)\n", httpd_cfg.PORT);
        exit(EXIT_FAILURE);
    }

    listenfd = create_and_bind(httpd_cfg.PORT);
    if (listenfd == -1) exit(EXIT_FAILURE);
    set_nonblocking(listenfd);

    if (listen(listenfd, SOMAXCONN) < 0) { // BACKLOG as SOMAXCONN, defined in linux kernel
        perror("Error: Socket listen failed \n Reason");
        exit(EXIT_FAILURE);
    }

    epollmgr_init(listenfd);

}

int httpd_shutdown_main() {

    close(listenfd);
    laji_log_close();
    
}