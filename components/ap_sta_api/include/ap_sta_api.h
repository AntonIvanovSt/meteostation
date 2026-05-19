#ifndef AP_STA_API_H_
#define AP_STA_API_H_

#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
// Event group to signal STA connection result
extern EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

httpd_handle_t start_webserver(void);
void wifi_init_softap(void);
void nvs_init(void);

#endif // !AP_STA_API_H_
