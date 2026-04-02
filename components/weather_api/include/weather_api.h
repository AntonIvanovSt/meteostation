#ifndef WEATHER_API_H_
#define WEATHER_API_H_

#define WEATHER_READY BIT0
#define WEATHER_FALL_BIT BIT1
#include "esp_http_client.h"

extern EventGroupHandle_t s_weather_event_group;

typedef struct {
    float temperature;
    float feels_like;
    int humidity;
    char condition[64];
    float wind_speed;
    uint64_t updated_at;
} weather_data_t;

void weather_task(void *pvParameters);

#endif // !WEATHER_API_H_
