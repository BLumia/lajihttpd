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
    
    cfg->ENABLE_LOG = 1;
    cfg->PORT = 8080;
    cfg->LOG_LEVEL = 'V';
    cfg->ENABLE_CACHE = 1;
    cfg->USE_MQ = 1;
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
        } else if (strcmp("ENABLE_LOG", laji_conf_get_varname()) == 0) {
            laji_conf_get_variable(&intval);
            cfg->ENABLE_LOG = intval;
        } else if (strcmp("LOG_LEVEL", laji_conf_get_varname()) == 0) {
            laji_conf_get_variable(&charval);
            cfg->LOG_LEVEL = charval;
        } else if (strcmp("ENABLE_CACHE", laji_conf_get_varname()) == 0) {
            laji_conf_get_variable(&intval);
            cfg->ENABLE_CACHE = intval;
        } else if (strcmp("USE_MQ", laji_conf_get_varname()) == 0) {
            laji_conf_get_variable(&intval);
            cfg->USE_MQ = intval;
        }
    }

    laji_conf_close();
}

int httpd_do_configuration(httpd_config_t* cfg) {

    laji_log_mq_toggle(cfg->USE_MQ);
    // enable log?
    http_caching_toggle(cfg->ENABLE_CACHE);
    laji_log_level_set_c(cfg->LOG_LEVEL);

}