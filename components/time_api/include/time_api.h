#ifndef TIME_API_H_
#define TIME_API_H_

#include "esp_sntp.h"
#include "time.h"
#include "time_api.h"

#define TIME_READY BIT0
#define TIME_FALL_BIT BIT1

extern EventGroupHandle_t s_time_event_group;

typedef struct {
    time_t timestamp;
    struct tm timeinfo;
} time_data_t;

void time_task(void *pvParameters);
void init_time(void);

#endif // !TIME_API_H_
