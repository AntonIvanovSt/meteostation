#ifndef WIFI_CONNECT_H_
#define WIFI_CONNECT_H_

#include "esp_wifi.h"
#include "nvs_flash.h"

#define WIFI_SSID "POCO X3 Pro"
#define WIFI_PASSWORD "27112711"
#define WIFI_AUTHMODE WIFI_AUTH_WPA2_PSK

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define WIFI_READY BIT2

extern EventGroupHandle_t s_wifi_event_group;

void wifi_connection_task();

#endif // !WIFI_CONNECT_H_
