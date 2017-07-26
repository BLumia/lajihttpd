#ifndef __LAJIHTTPD_CFG_FILE
#define __LAJIHTTPD_CFG_FILE

#define PATH_MAX 4096
#define BUFFER_SIZE 8192

typedef struct httpd_config {
    char WWW_PATH[PATH_MAX];
    int PORT;
    char LOG_LEVEL;
    int ENABLE_CACHE;
    int USE_MQ;
} httpd_config_t;

int httpd_config_main(httpd_config_t* cfg);

#endif /* __LAJIHTTPD_CFG_FILE */