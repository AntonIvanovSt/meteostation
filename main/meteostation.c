#include "ap_sta_api.h"
#include "display/lv_display.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "lv_api_map_v8.h"
#include "lvgl_api.h"
#include "scd41_api.h"
#include "time_api.h"
#include "weather_api.h"
#include "widgets/label/lv_label.h"

// screens
static lv_obj_t *screen_start = NULL;
static lv_obj_t *screen_sensor = NULL;
// labels
static lv_obj_t *temp_label = NULL;
static lv_obj_t *humidity_label = NULL;
static lv_obj_t *co2_label = NULL;
static lv_obj_t *temp_label_ = NULL;
static lv_obj_t *humidity_label_ = NULL;
static lv_obj_t *info_label = NULL;
static lv_obj_t *time_label = NULL;
static lv_obj_t *date_label = NULL;
static lv_obj_t *out_temp_l = NULL;
static lv_obj_t *out_feels_like_l = NULL;
static lv_obj_t *out_wind_l = NULL;
static lv_obj_t *out_hum_l = NULL;
static lv_obj_t *out_condition_l = NULL;

static const char *TAG = "MAIN";

void load_screen(lv_obj_t *screen) {
    if (lvgl_port_lock(pdMS_TO_TICKS(100))) {
        lv_screen_load(screen);
        lvgl_port_unlock();
    }
}

void time_init_task(void *pvParameters) {
    init_time();
    vTaskDelete(NULL);
}

void scd41_init_task(void *pvParameters) {
    init_scd41();
    vTaskDelete(NULL);
}

// START SCREEN
void create_start_screen(void) {
    extern lv_font_t oswald_regular_20;
    screen_start = create_background(COLOR_WHITE);

    if (screen_start != NULL) {
        info_label = create_label(
            screen_start, &oswald_regular_20, COLOR_BLACK, 10, 30,
            "- Disconnect from any network\n- Connect to esp_soft_ap\n- Search "
            "in your browser:\nhttp:\\\\192.168.4.1\\");
    }
}
// END OF START SCREEN

// SENSOR SCREEN
void create_sensor_screen(void) {
    extern lv_font_t oswald_medium_48;
    extern lv_font_t oswald_medium_20;
    extern lv_font_t oswald_regular_20;
    screen_sensor = create_background(COLOR_WHITE);
    if (screen_sensor != NULL) {
        // time labels
        time_label = create_label(screen_sensor, &oswald_medium_48, COLOR_BLACK,
                                  10, 10, "00:00");
        date_label = create_label(screen_sensor, &oswald_regular_20,
                                  COLOR_BLACK, 13, 60, "YYYY/mm/dd");
        // --------
        // scd41 labels
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 10, 90,
                     "TEMPERATURE");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 60, 113,
                     "°C");
        temp_label = create_label(screen_sensor, &oswald_medium_48, COLOR_BLACK,
                                  10, 115, "");
        temp_label_ = create_label(screen_sensor, &oswald_medium_20,
                                   COLOR_BLACK, 60, 135, "");

        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 10, 165,
                     "HUMIDITY");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 60, 188,
                     "%");
        humidity_label = create_label(screen_sensor, &oswald_medium_48,
                                      COLOR_BLACK, 10, 190, "");
        humidity_label_ = create_label(screen_sensor, &oswald_medium_20,
                                       COLOR_BLACK, 60, 210, "");

        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 10, 240,
                     "CO2 (ppm)");
        co2_label = create_label(screen_sensor, &oswald_medium_48, COLOR_BLACK,
                                 10, 265, "");
        // --------
        // weatherapi labels
        out_condition_l = create_label(screen_sensor, &oswald_medium_20,
                                       COLOR_BLACK, 170, 10, "");

        out_temp_l = create_label(screen_sensor, &oswald_medium_48, COLOR_BLACK,
                                  170, 30, "");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 210, 28,
                     "°C");

        out_feels_like_l = create_label(screen_sensor, &oswald_medium_48,
                                        COLOR_BLACK, 170, 115, "");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 210, 113,
                     "°C");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 120, 90,
                     "FEELS LIKE");

        out_hum_l = create_label(screen_sensor, &oswald_medium_48, COLOR_BLACK,
                                 170, 190, "");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 210, 188,
                     "%");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 120, 165,
                     "OUT HUMIDITY");

        out_wind_l = create_label(screen_sensor, &oswald_medium_48, COLOR_BLACK,
                                  170, 265, "");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 120, 240,
                     "WIND (km/h)");
    }
}

void update_time_data(time_data_t data) {
    if (time_label == NULL || date_label == NULL) {
        ESP_LOGE(TAG, "Time was not initialized");
        return;
    }
    char time_buffer[64];
    if (lvgl_port_lock(pdMS_TO_TICKS(100))) {
        strftime(time_buffer, sizeof(time_buffer), "%I:%M", &data.timeinfo);
        lv_label_set_text(time_label, time_buffer);
        strftime(time_buffer, sizeof(time_buffer), "%Y/%m/%d", &data.timeinfo);
        lv_label_set_text(date_label, time_buffer);
        lvgl_port_unlock();
    }
}

void update_sensor_data(float co2, float temperature, float humidity) {
    if (temp_label == NULL || humidity_label == NULL || co2_label == NULL) {
        ESP_LOGE(TAG, "Sensor labels not initialized");
        return;
    }

    char buf[32];

    if (!lvgl_port_lock(pdMS_TO_TICKS(100))) {
        ESP_LOGW(TAG, "Could not acquire LVGL lock, skipping update");
        return;
    }

    snprintf(buf, sizeof(buf), "%d", (int)temperature);
    lv_label_set_text(temp_label, buf);
    if (temp_label_ != NULL) {
        snprintf(buf, sizeof(buf), ".%d", abs((int)(temperature * 10.0f) % 10));
        lv_label_set_text(temp_label_, buf);
    }

    snprintf(buf, sizeof(buf), "%d", (int)humidity);
    lv_label_set_text(humidity_label, buf);
    if (humidity_label_ != NULL) {
        snprintf(buf, sizeof(buf), ".%d", abs((int)(humidity * 10.0f) % 10));
        lv_label_set_text(humidity_label_, buf);
    }

    snprintf(buf, sizeof(buf), "%d", (int)co2);
    lv_label_set_text(co2_label, buf);

    lvgl_port_unlock();
}

void update_weather_data(weather_data_t data) {
    if (out_temp_l == NULL || out_condition_l == NULL ||
        out_feels_like_l == NULL || out_hum_l == NULL || out_wind_l == NULL) {
        ESP_LOGE(TAG, "Weather was not fetched");
        return;
    }
    ESP_LOGI(TAG, "Updating weather: %.1f°C, %s", data.temperature,
             data.condition);
    char weather_buffer[64];
    if (lvgl_port_lock(pdMS_TO_TICKS(100))) {
        snprintf(weather_buffer, sizeof(weather_buffer), "%.1f",
                 data.temperature);
        lv_label_set_text(out_temp_l, weather_buffer);

        snprintf(weather_buffer, sizeof(weather_buffer), "%d", data.humidity);
        lv_label_set_text(out_hum_l, weather_buffer);

        snprintf(weather_buffer, sizeof(weather_buffer), "%s", data.condition);
        lv_label_set_text(out_condition_l, weather_buffer);

        snprintf(weather_buffer, sizeof(weather_buffer), "%.1f",
                 data.feels_like);
        lv_label_set_text(out_feels_like_l, weather_buffer);

        snprintf(weather_buffer, sizeof(weather_buffer), "%.1f",
                 data.wind_speed);
        lv_label_set_text(out_wind_l, weather_buffer);
        lvgl_port_unlock();
        ESP_LOGI(TAG, "Weather labels updated");
    } else {
        ESP_LOGW(TAG, "LVGL lock failed in update_weather_data");
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

void d_scd41_task(void *pvParameters) {
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    scd41_custom_t data;

    while (1) {
        if (xQueueReceive(queue, &data, pdMS_TO_TICKS(10000)) == pdTRUE) {
            update_sensor_data(data.co2, data.temperature, data.humidity);
        }
    }
}

void d_weather_task(void *pvParameters) {
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    weather_data_t data;
    ESP_LOGI(TAG, "d_weather_task started");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(500));
        if (xQueueReceive(queue, &data, pdMS_TO_TICKS(10000)) == pdTRUE) {
            ESP_LOGI(
                TAG,
                "Weather data received from queue"); // ← is queue delivering?
            update_weather_data(data);
        } else {
            ESP_LOGW(TAG, "Weather queue timeout"); // ← is it timing out?
        }
    }
}

// END OF SENSOR SCREEN

bool stage_wifi(void) {
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi ready");
        if (lvgl_port_lock(pdMS_TO_TICKS(100))) {
            lv_label_set_text(info_label, "WIFI initialized");
            lvgl_port_unlock();
        }
        return true;
    }
    if (bits & WIFI_FAIL_BIT) {
        ESP_LOGW(TAG, "WiFi failed, continuing without network");
        if (lvgl_port_lock(pdMS_TO_TICKS(100))) {
            lv_label_set_text(info_label, "WIFI failed");
            lvgl_port_unlock();
        }
    }
    return false;
}

bool stage_time(QueueHandle_t time_queue) {
    if (lvgl_port_lock(pdMS_TO_TICKS(100))) {
        lv_label_set_text(info_label, "Syncing time...");
        lvgl_port_unlock();
    }
    xTaskCreate(time_init_task, "time_init", 4096, NULL, 5, NULL);

    EventBits_t bits =
        xEventGroupWaitBits(s_time_event_group, TIME_READY | TIME_FALL_BIT,
                            pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & TIME_READY) {
        ESP_LOGI(TAG, "Time ready");
        xTaskCreate(time_task, "time_task", 4096, time_queue, 5, NULL);
        return true;
    }
    if (bits & TIME_FALL_BIT) {
        ESP_LOGW(TAG, "Time sync failed, continuing without time");
    }
    return false;
}

bool stage_scd41(QueueHandle_t scd41_queue) {
    if (lvgl_port_lock(pdMS_TO_TICKS(100))) {
        lv_label_set_text(info_label, "Initializing sensor...");
        lvgl_port_unlock();
    }
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

bool stage_weather(QueueHandle_t weather_queue) {
    if (lvgl_port_lock(pdMS_TO_TICKS(100))) {
        lv_label_set_text(info_label, "Fetching weather...");
        lvgl_port_unlock();
    }
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

static void create_task_checked(TaskFunction_t fn, const char *name,
                                uint32_t stack, void *param, UBaseType_t prio) {
    BaseType_t ret = xTaskCreate(fn, name, stack, param, prio, NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create task '%s' (err=%d, free heap=%lu)",
                 name, ret, esp_get_free_heap_size());
    } else {
        ESP_LOGI(TAG, "Task '%s' created (free heap=%lu)", name,
                 esp_get_free_heap_size());
    }
}

// ─── Main ────────────────────────────────────────────────────────────────────

void app_main(void) {
    QueueHandle_t time_queue = xQueueCreate(1, sizeof(time_data_t));
    QueueHandle_t scd41_queue = xQueueCreate(1, sizeof(scd41_custom_t));
    QueueHandle_t weather_queue = xQueueCreate(1, sizeof(weather_data_t));

    s_wifi_event_group = xEventGroupCreate();
    s_scd41_event_group = xEventGroupCreate();
    s_time_event_group = xEventGroupCreate();
    s_weather_event_group = xEventGroupCreate();

    init_lvgl(LV_DISP_ROTATION_0);

    create_start_screen();

    nvs_init();
    wifi_init_softap();
    start_webserver();

    stage_wifi();

    create_sensor_screen();
    load_screen(screen_sensor);

    if (stage_time(time_queue)) {
        create_task_checked(d_time_task, "time_rx_task", 2048, time_queue, 4);
    }
    if (stage_scd41(scd41_queue)) {
        create_task_checked(d_scd41_task, "scd41_rx_task", 2048, scd41_queue,
                            4);
    }
    if (stage_weather(weather_queue)) {
        create_task_checked(d_weather_task, "weather_rx_task", 3072,
                            weather_queue, 4);
    }

    load_screen(screen_start);
    vTaskDelete(NULL);
}
