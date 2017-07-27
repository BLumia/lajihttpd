#include <string.h>
#include <configman.h>
#include <logger.h>
#include "http_utils.h"
#include "config.h"

int httpd_init_configure(httpd_config_t* cfg);
int httpd_process_configfile(httpd_config_t* cfg);
int httpd_do_configuration(httpd_config_t* cfg);

int httpd_config_main(httpd_config_t* cfg) {

    httpd_init_configure(cfg);
    httpd_process_configfile(cfg);
    httpd_do_configuration(cfg);

}

int httpd_init_configure(httpd_config_t* cfg) {
    
    cfg->PORT = 8080;
    cfg->LOG_LEVEL = 'V';
    cfg->ENABLE_CACHE = 1;
    cfg->USE_MQ = 1;
    cfg->KEEP_ALIVE_SUPPORT = 0;
}

int httpd_process_configfile(httpd_config_t* cfg) {

    int intval;
    char charval;
    char strval[616];

    laji_conf_open("./httpd.conf");

    while (laji_conf_next_variable() == 0) {
        if (strcmp("WWW_PATH", laji_conf_get_varname()) == 0) {
            laji_conf_get_variable(strval);
            strcpy(cfg->WWW_PATH, strval);
        } else if (strcmp("PORT", laji_conf_get_varname()) == 0) {
            laji_conf_get_variable(&intval);
            cfg->PORT = intval;
        } else if (strcmp("LOG_LEVEL", laji_conf_get_varname()) == 0) {
            laji_conf_get_variable(&charval);
            cfg->LOG_LEVEL = charval;
        } else if (strcmp("ENABLE_CACHE", laji_conf_get_varname()) == 0) {
            laji_conf_get_variable(&intval);
            cfg->ENABLE_CACHE = intval;
        } else if (strcmp("USE_MQ", laji_conf_get_varname()) == 0) {
            laji_conf_get_variable(&intval);
            cfg->USE_MQ = intval;
        } else if (strcmp("KEEP_ALIVE_SUPPORT", laji_conf_get_varname())) {
            laji_conf_get_variable(&intval);
            cfg->KEEP_ALIVE_SUPPORT = intval;
        }
    }

    laji_conf_close();
}

int httpd_do_configuration(httpd_config_t* cfg) {

    laji_log_mq_toggle(cfg->USE_MQ);
    http_caching_toggle(cfg->ENABLE_CACHE);
    laji_log_level_set_c(cfg->LOG_LEVEL);
    http_keepalive_toggle(cfg->KEEP_ALIVE_SUPPORT);

    return 0;
}