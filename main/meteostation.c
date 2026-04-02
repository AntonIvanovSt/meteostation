#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "lvgl_api.h"
#include "misc/lv_types.h"
#include "scd41_api.h"
#include "time_api.h"
#include "weather_api.h"
#include "wifi_connect.h"
#include <stdio.h>
#include <time.h>

// screens
static lv_obj_t *screen_start = NULL;
static lv_obj_t *screen_sensor = NULL;
// labels
static lv_obj_t *temp_label = NULL;
static lv_obj_t *humidity_label = NULL;
static lv_obj_t *co2_label = NULL;
static lv_obj_t *info_label = NULL;
static lv_obj_t *time_label = NULL;
static lv_obj_t *date_label = NULL;

static const char *TAG = "MAIN";

// START SCREEN
void create_start_screen(void) {
    extern lv_font_t kode_mono_24;
    screen_start = create_background(COLOR_BLACK);

    if (screen_start != NULL) {
        info_label = create_label(screen_start, &kode_mono_24, COLOR_WHITE, 20,
                                  30, "Initializing WIFI...");
    }
}
// END OF START SCREEN

// SENSOR SCREEN
void create_sensor_screen(void) {
    extern lv_font_t kode_mono_24;
    screen_sensor = create_background(COLOR_BLACK);
    if (screen_sensor != NULL) {
        temp_label = create_label(screen_sensor, &kode_mono_24, COLOR_WHITE, 20,
                                  120, "T: --");
        humidity_label = create_label(screen_sensor, &kode_mono_24, COLOR_WHITE,
                                      20, 150, "H: --");
        co2_label = create_label(screen_sensor, &kode_mono_24, COLOR_WHITE, 20,
                                 180, "C: --");
        time_label = create_label(screen_sensor, &kode_mono_24, COLOR_CYAN, 20,
                                  30, "00:00");
        date_label = create_label(screen_sensor, &kode_mono_24, COLOR_CYAN, 20,
                                  60, "YYYY/mm/dd");
    }
}

void update_sensor_data(float co2, float temperature, float humidity) {
    if (temp_label == NULL || humidity_label == NULL || co2_label == NULL) {
        ESP_LOGE(TAG, "Sensor was not initialized");
        return;
    }
    char sensor_buffer[64];
    if (lvgl_port_lock(0)) {
        snprintf(sensor_buffer, sizeof(sensor_buffer), "T: %.1f C",
                 temperature);
        lv_label_set_text(temp_label, sensor_buffer);
        snprintf(sensor_buffer, sizeof(sensor_buffer), "H: %.1f %%", humidity);
        lv_label_set_text(humidity_label, sensor_buffer);
        snprintf(sensor_buffer, sizeof(sensor_buffer), "C: %d ppm", (int)co2);
        lv_label_set_text(co2_label, sensor_buffer);
        lvgl_port_unlock();
    }
}

void update_time_data(time_data_t data) {
    if (time_label == NULL || date_label == NULL) {
        ESP_LOGE(TAG, "Time was not initialized");
        return;
    }
    char time_buffer[64];
    if (lvgl_port_lock(0)) {
        strftime(time_buffer, sizeof(time_buffer), "%I:%M %p", &data.timeinfo);
        lv_label_set_text(time_label, time_buffer);
        strftime(time_buffer, sizeof(time_buffer), "%Y/%m/%d", &data.timeinfo);
        lv_label_set_text(date_label, time_buffer);
        lvgl_port_unlock();
    }
}

void scd41_init_task(void *pvParameters) {
    init_scd41();
    vTaskDelete(NULL);
}

void time_init_task(void *pvParameters) {
    init_time();
    vTaskDelete(NULL);
}

void d_scd41_task(void *pvParameters) {
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    scd41_custom_t data;

    while (1) {
        if (xQueueReceive(queue, &data, pdMS_TO_TICKS(10000)) == pdTRUE) {
            update_sensor_data(data.co2, data.temperature, data.humidity);
        }
    }
}

void d_time_task(void *pvParameters) {
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    time_data_t data;

    while (1) {
        if (xQueueReceive(queue, &data, pdMS_TO_TICKS(10000)) == pdTRUE) {
            update_time_data(data);
        }
    }
}

void d_weather_task(void *pvParameters) {
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    weather_data_t data;

    while (1) {
        if (xQueueReceive(queue, &data, pdMS_TO_TICKS(10000)) == pdTRUE) {
            ESP_LOGI(TAG, "weather data is: %d", data.temperature);
        }
    }
}
// END OF SENSOR SCREEN

void load_screen(lv_obj_t *screen) {
    if (lvgl_port_lock(0)) {
        lv_screen_load(screen);
        lvgl_port_unlock();
    }
}

void app_main(void) {

    QueueHandle_t scd41_queue = xQueueCreate(1, sizeof(scd41_custom_t));
    QueueHandle_t time_queue = xQueueCreate(1, sizeof(time_data_t));
    QueueHandle_t weather_queue = xQueueCreate(1, sizeof(weather_data_t));

    s_wifi_event_group = xEventGroupCreate();
    s_scd41_event_group = xEventGroupCreate();
    s_time_event_group = xEventGroupCreate();

    init_lvgl(LV_DISP_ROT_90);

    create_start_screen();
    load_screen(screen_start);

    xTaskCreate(wifi_connection_task, "wifi_task", 8192, NULL, 5, NULL);

    EventBits_t wifi_bits =
        xEventGroupWaitBits(s_wifi_event_group,         // the event group
                            WIFI_READY | WIFI_FAIL_BIT, // bits to watch
                            pdFALSE,      // don't clear bits after returning
                            pdFALSE,      // wait for ANY bit
                            portMAX_DELAY // wait forever
        );

    if (wifi_bits & WIFI_READY) {
        lv_label_set_text(info_label, "Initializing sensor...");
        xTaskCreate(scd41_init_task, "scd41_init", 4096, NULL, 5, NULL);
    }
    if (wifi_bits & WIFI_FAIL_BIT) {
        lv_label_set_text(info_label,
                          "Can not connect to WIFI\nrunning sensor only");
        xTaskCreate(scd41_init_task, "scd41_init", 4096, NULL, 5, NULL);
    }
    EventBits_t scd41_bits =
        xEventGroupWaitBits(s_scd41_event_group, // the event group
                            SCD41_READY,         // bits to watch
                            pdFALSE,      // don't clear bits after returning
                            pdTRUE,       // wait for all bit
                            portMAX_DELAY // wait forever
        );

    if ((scd41_bits & SCD41_READY) && (wifi_bits & WIFI_FAIL_BIT)) {
        create_sensor_screen();
        load_screen(screen_sensor);

        xTaskCreate(scd41_task, "scd41_task", 4096, scd41_queue, 5, NULL);
        xTaskCreate(d_scd41_task, "scd41_rx_task", 4096, scd41_queue, 4, NULL);
    }

    if ((scd41_bits & SCD41_READY) && (wifi_bits & WIFI_READY)) {
        lv_label_set_text(info_label, "Initializing time...");
        xTaskCreate(time_init_task, "time_init", 4096, NULL, 5, NULL);
    }
    EventBits_t time_bits =
        xEventGroupWaitBits(s_time_event_group, // the event group
                            TIME_READY,         // bits to watch
                            pdFALSE,      // don't clear bits after returning
                            pdTRUE,       // wait for all bit
                            portMAX_DELAY // wait forever
        );

    if (time_bits & TIME_READY) {
        create_sensor_screen();
        load_screen(screen_sensor);

        xTaskCreate(scd41_task, "scd41_task", 4096, scd41_queue, 5, NULL);
        xTaskCreate(d_scd41_task, "scd41_rx_task", 4096, scd41_queue, 4, NULL);
        xTaskCreate(time_task, "time_task", 8192, time_queue, 5, NULL);
        xTaskCreate(d_time_task, "time_rx_task", 4096, time_queue, 4, NULL);
        xTaskCreate(weather_task, "weather_task", 8192, weather_queue, 5, NULL);
        xTaskCreate(d_weather_task, "weather_rx_task", 4096, weather_queue, 4,
                    NULL);
    }
    vTaskDelete(NULL);
}
