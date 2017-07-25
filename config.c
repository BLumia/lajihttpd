#include <string.h>
#include <configman.h>
#include "config.h"

int httpd_config_main(httpd_config_t* cfg) {

    httpd_init_configure(cfg);
    httpd_process_configfile(cfg);

}

int httpd_init_configure(httpd_config_t* cfg) {
    
    cfg->ENABLE_LOG = 1;
    cfg->PORT = 8080;

}

int httpd_process_configfile(httpd_config_t* cfg) {

    int intval;
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
        }
    }

    laji_conf_close();
}
