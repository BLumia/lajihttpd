#ifndef __LAJIHTTPD_CFG_FILE
#define __LAJIHTTPD_CFG_FILE

#define PATH_MAX 4096
#define BUFFER_SIZE 8192

typedef struct httpd_config {
    char WWW_PATH[PATH_MAX];
    int PORT;
    int ENABLE_LOG;
} httpd_config_t;

int httpd_config_main(httpd_config_t* cfg);
int httpd_init_configure(httpd_config_t* cfg);
int httpd_process_configfile(httpd_config_t* cfg);

#endif /* __LAJIHTTPD_CFG_FILE */