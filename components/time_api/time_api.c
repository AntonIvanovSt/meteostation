
#include "time_api.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"

static const char *TAG = "time_api";

EventGroupHandle_t s_time_event_group = NULL;

static void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "Time synchronized!");
    xEventGroupSetBits(s_time_event_group, TIME_READY);
}

void init_time(void) {
    setenv("TZ", "JST-9", 1);
    tzset();
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();

    int retry = 0;
    while (esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && retry < 10) {
        ESP_LOGI(TAG, "Waiting for time sync... (%d/10)", retry++);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    if (esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && retry > 10) {
        ESP_LOGE(TAG, "Time sync failed after 10 retries");
        xEventGroupSetBits(s_time_event_group, TIME_FALL_BIT);
    }
}

void time_task(void *pvParameters) {
    QueueHandle_t time_queue = (QueueHandle_t)pvParameters;
    time_data_t data;

    while (1) {
        time(&data.timestamp);
        localtime_r(&data.timestamp, &data.timeinfo);
        xQueueOverwrite(time_queue, &data);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
