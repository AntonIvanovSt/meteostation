#ifndef AP_STA_API_H_
#define AP_STA_API_H_

#include "esp_http_server.h"
#include "freertos/event_groups.h"

extern EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

httpd_handle_t start_webserver(void);
void nvs_init(void);
bool wifi_start_auto(void);

#endif // !AP_STA_API_H_
