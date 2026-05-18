#ifndef AP_STA_API_H_
#define AP_STA_API_H_

#include "esp_http_server.h"

httpd_handle_t start_webserver(void);
void wifi_init_softap(void);
void nvs_init(void);

#endif // !AP_STA_API_H_
