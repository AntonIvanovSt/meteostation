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
static lv_obj_t *out_temp_l = NULL;
static lv_obj_t *out_feels_like_l = NULL;
static lv_obj_t *out_wind_l = NULL;
static lv_obj_t *out_hum_l = NULL;
static lv_obj_t *out_condition_l = NULL;

static const char *TAG = "MAIN";

// START SCREEN
void create_start_screen(void) {
    extern lv_font_t kode_mono_20;
    screen_start = create_background(COLOR_WHITE);

    if (screen_start != NULL) {
        info_label = create_label(screen_start, &kode_mono_20, COLOR_BLACK, 20,
                                  30, "Initializing WIFI...");
    }
}
// END OF START SCREEN

// SENSOR SCREEN
void create_sensor_screen(void) {
    extern lv_font_t kode_mono_20;
    screen_sensor = create_background(COLOR_WHITE);
    if (screen_sensor != NULL) {
        temp_label = create_label(screen_sensor, &kode_mono_20, COLOR_BLACK, 20,
                                  120, "T: --");
        humidity_label = create_label(screen_sensor, &kode_mono_20, COLOR_BLACK,
                                      20, 150, "H: --");
        co2_label = create_label(screen_sensor, &kode_mono_20, COLOR_BLACK, 20,
                                 180, "C: --");
        time_label = create_label(screen_sensor, &kode_mono_20, COLOR_BLACK, 20,
                                  30, "00:00");
        date_label = create_label(screen_sensor, &kode_mono_20, COLOR_BLACK, 20,
                                  60, "YYYY/mm/dd");
        out_temp_l = create_label(screen_sensor, &kode_mono_20, COLOR_BLACK,
                                  170, 30, "T: --");
        out_hum_l = create_label(screen_sensor, &kode_mono_20, COLOR_BLACK, 170,
                                 90, "H: --");
        out_feels_like_l = create_label(screen_sensor, &kode_mono_20,
                                        COLOR_BLACK, 170, 60, "(---)");
        out_condition_l = create_label(screen_sensor, &kode_mono_20,
                                       COLOR_BLACK, 170, 150, "---");
        out_wind_l = create_label(screen_sensor, &kode_mono_20, COLOR_BLACK,
                                  170, 120, "W: --");
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

void update_weather_data(weather_data_t data) {
    if (out_temp_l == NULL || out_condition_l == NULL ||
        out_feels_like_l == NULL || out_hum_l == NULL || out_wind_l == NULL) {
        ESP_LOGE(TAG, "Weather was not fetched");
        return;
    }
    char weather_buffer[64];
    if (lvgl_port_lock(0)) {
        snprintf(weather_buffer, sizeof(weather_buffer), "T: %.1f C",
                 data.temperature);
        lv_label_set_text(out_temp_l, weather_buffer);

        snprintf(weather_buffer, sizeof(weather_buffer), "H: %d %%",
                 data.humidity);
        lv_label_set_text(out_hum_l, weather_buffer);

        snprintf(weather_buffer, sizeof(weather_buffer), "%s", data.condition);
        lv_label_set_text(out_condition_l, weather_buffer);

        snprintf(weather_buffer, sizeof(weather_buffer), "(%.1f C)",
                 data.feels_like);
        lv_label_set_text(out_feels_like_l, weather_buffer);

        snprintf(weather_buffer, sizeof(weather_buffer), "W: %.1f km/h",
                 data.wind_speed);
        lv_label_set_text(out_wind_l, weather_buffer);
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
            update_weather_data(data);
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

bool stage_wifi(void) {
    lv_label_set_text(info_label, "Connecting to WiFi...");
    xTaskCreate(wifi_connection_task, "wifi_task", 8192, NULL, 5, NULL);

    EventBits_t bits =
        xEventGroupWaitBits(s_wifi_event_group, WIFI_READY | WIFI_FAIL_BIT,
                            pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_READY) {
        ESP_LOGI(TAG, "WiFi ready");
        return true;
    }
    if (bits & WIFI_FAIL_BIT) {
        ESP_LOGW(TAG, "WiFi failed, continuing without network");
    }
    return false;
}

bool stage_scd41(QueueHandle_t scd41_queue) {
    lv_label_set_text(info_label, "Initializing sensor...");
    xTaskCreate(scd41_init_task, "scd41_init", 4096, NULL, 5, NULL);

    EventBits_t bits =
        xEventGroupWaitBits(s_scd41_event_group, SCD41_READY | SCD41_FALL_BIT,
                            pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & SCD41_READY) {
        ESP_LOGI(TAG, "SCD41 ready");
        xTaskCreate(scd41_task, "scd41_task", 4096, scd41_queue, 5, NULL);
        return true;
    }
    if (bits & SCD41_FALL_BIT) {
        ESP_LOGE(TAG, "SCD41 init failed");
    }
    return false;
}

bool stage_time(QueueHandle_t time_queue) {
    lv_label_set_text(info_label, "Syncing time...");
    xTaskCreate(time_init_task, "time_init", 4096, NULL, 5, NULL);

    EventBits_t bits =
        xEventGroupWaitBits(s_time_event_group, TIME_READY | TIME_FALL_BIT,
                            pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & TIME_READY) {
        ESP_LOGI(TAG, "Time ready");
        xTaskCreate(time_task, "time_task", 8192, time_queue, 5, NULL);
        return true;
    }
    if (bits & TIME_FALL_BIT) {
        ESP_LOGW(TAG, "Time sync failed, continuing without time");
    }
    return false;
}

bool stage_weather(QueueHandle_t weather_queue) {
    lv_label_set_text(info_label, "Fetching weather...");
    xTaskCreate(weather_task, "weather_task", 8192, weather_queue, 5, NULL);

    EventBits_t bits = xEventGroupWaitBits(s_weather_event_group,
                                           WEATHER_READY | WEATHER_FALL_BIT,
                                           pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WEATHER_READY) {
        ESP_LOGI(TAG, "Weather ready");
        return true;
    }
    if (bits & WEATHER_FALL_BIT) {
        ESP_LOGW(TAG, "Weather fetch failed, continuing without weather");
    }
    return false;
}

void launch_display_tasks(QueueHandle_t scd41_queue, QueueHandle_t time_queue,
                          QueueHandle_t weather_queue, bool scd41_ok,
                          bool time_ok, bool weather_ok) {
    create_sensor_screen();
    load_screen(screen_sensor);

    if (scd41_ok) {
        xTaskCreate(d_scd41_task, "scd41_rx_task", 4096, scd41_queue, 4, NULL);
    }
    if (time_ok) {
        xTaskCreate(d_time_task, "time_rx_task", 4096, time_queue, 4, NULL);
    }
    if (weather_ok) {
        xTaskCreate(d_weather_task, "weather_rx_task", 4096, weather_queue, 4,
                    NULL);
    }
}

void app_main(void) {
    QueueHandle_t scd41_queue = xQueueCreate(1, sizeof(scd41_custom_t));
    QueueHandle_t time_queue = xQueueCreate(1, sizeof(time_data_t));
    QueueHandle_t weather_queue = xQueueCreate(1, sizeof(weather_data_t));

    s_wifi_event_group = xEventGroupCreate();
    s_scd41_event_group = xEventGroupCreate();
    s_time_event_group = xEventGroupCreate();
    s_weather_event_group = xEventGroupCreate();

    init_lvgl(LV_DISP_ROT_90);
    create_start_screen();
    load_screen(screen_start);

    bool wifi_ok = stage_wifi();
    bool scd41_ok = stage_scd41(scd41_queue);
    if (!scd41_ok) {
        lv_label_set_text(info_label, "Sensor error!\nCannot continue.");
        ESP_LOGE(TAG, "SCD41 is required, halting");
        vTaskDelete(NULL);
    }
    if (!wifi_ok) {
        lv_label_set_text(info_label, "No WiFi\nSensor only mode");
        launch_display_tasks(scd41_queue, NULL, NULL, true, false, false);
        vTaskDelete(NULL);
    }
    bool time_ok = stage_time(time_queue);
    if (!time_ok) {
        lv_label_set_text(info_label, "Time syncing error.");
    }
    bool weather_ok = stage_weather(weather_queue);
    if (!weather_ok) {
        lv_label_set_text(info_label, "Weather fetching error.");
    }

    launch_display_tasks(scd41_queue, time_queue, weather_queue, scd41_ok,
                         time_ok, weather_ok);
    vTaskDelete(NULL);
}
