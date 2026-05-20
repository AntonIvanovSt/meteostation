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
static lv_obj_t *out_temp_l_ = NULL;
static lv_obj_t *out_feels_like_l_ = NULL;
static lv_obj_t *out_wind_l_ = NULL;
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
        info_label = create_label(screen_start, &oswald_regular_20, COLOR_BLACK,
                                  10, 30, "");
    }
}
// END OF START SCREEN

// SENSOR SCREEN
void create_sensor_screen(void) {
    extern lv_font_t oswald_medium_48;
    extern lv_font_t oswald_medium_20;
    extern lv_font_t oswald_regular_20;

    static lv_point_precise_t horizontal_line_points[] = {
        {0, 0},  // Start point (x, y)
        {240, 0} // End point (x, y)
    };
    static lv_point_precise_t vertical_line_points[] = {
        {0, 0},  // Start point (x, y)
        {0, 234} // End point (x, y)
    };
    static lv_point_precise_t vertical_short_line_points[] = {
        {0, 0}, // Start point (x, y)
        {0, 86} // End point (x, y)
    };
    screen_sensor = create_background(COLOR_WHITE);

    create_line(horizontal_line_points, screen_sensor, COLOR_BLACK, 0, 86);
    create_line(vertical_line_points, screen_sensor, COLOR_BLACK, 120, 86);
    create_line(vertical_short_line_points, screen_sensor, COLOR_BLACK, 135, 0);

    if (screen_sensor != NULL) {
        // time labels
        time_label = create_label(screen_sensor, &oswald_medium_48, COLOR_BLACK,
                                  10, 10, "00:00");
        date_label = create_label(screen_sensor, &oswald_regular_20,
                                  COLOR_BLACK, 13, 60, "YYYY/mm/dd");
        // --------
        // scd41 labels
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 5, 90,
                     "TEMPERATURE");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 60, 113,
                     "°C");
        temp_label = create_label(screen_sensor, &oswald_medium_48, COLOR_BLACK,
                                  10, 115, "00");
        temp_label_ = create_label(screen_sensor, &oswald_medium_20,
                                   COLOR_BLACK, 60, 137, ".0");

        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 5, 165,
                     "HUMIDITY");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 60, 188,
                     "%");
        humidity_label = create_label(screen_sensor, &oswald_medium_48,
                                      COLOR_BLACK, 10, 190, "00");
        humidity_label_ = create_label(screen_sensor, &oswald_medium_20,
                                       COLOR_BLACK, 60, 212, ".0");

        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 5, 240,
                     "CO2 (ppm)");
        co2_label = create_label(screen_sensor, &oswald_medium_48, COLOR_BLACK,
                                 10, 265, "1000");
        // --------
        // weatherapi labels
        out_condition_l = create_label(screen_sensor, &oswald_regular_20,
                                       COLOR_BLACK, 140, 5, "Partly cloudy");

        out_temp_l = create_label(screen_sensor, &oswald_medium_48, COLOR_BLACK,
                                  145, 35, "25");
        out_temp_l_ = create_label(screen_sensor, &oswald_medium_20,
                                   COLOR_BLACK, 195, 57, ".3");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 195, 33,
                     "°C");

        out_feels_like_l = create_label(screen_sensor, &oswald_medium_48,
                                        COLOR_BLACK, 130, 115, "26");
        out_feels_like_l_ = create_label(screen_sensor, &oswald_medium_20,
                                         COLOR_BLACK, 180, 137, ".7");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 180, 113,
                     "°C");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 125, 90,
                     "FEELS LIKE");

        out_hum_l = create_label(screen_sensor, &oswald_medium_48, COLOR_BLACK,
                                 130, 190, "69");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 180, 188,
                     "%");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 125, 165,
                     "OUT HUMIDITY");

        out_wind_l = create_label(screen_sensor, &oswald_medium_48, COLOR_BLACK,
                                  155, 265, "4");
        out_wind_l_ = create_label(screen_sensor, &oswald_medium_20,
                                   COLOR_BLACK, 180, 287, ".5");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 125, 240,
                     "WIND");
        create_label(screen_sensor, &oswald_regular_20, COLOR_BLACK, 180, 263,
                     "km/h");
    }
}

void set_x_pos(int max_x, int min_x, lv_obj_t *label, int data) {
    int main_x_pos = (data < 10) ? max_x : min_x;
    lv_obj_set_x(label, main_x_pos);
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

    set_x_pos(35, 10, temp_label, (int)temperature);
    snprintf(buf, sizeof(buf), "%d", (int)temperature);
    lv_label_set_text(temp_label, buf);
    if (temp_label_ != NULL) {
        snprintf(buf, sizeof(buf), ".%d", abs((int)(temperature * 10.0f) % 10));
        lv_label_set_text(temp_label_, buf);
    }

    set_x_pos(35, 10, humidity_label, (int)humidity);
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
        ESP_LOGE(TAG, "Weather labels not initialized or weather not fetched");
        return;
    }

    char weather_buffer[64];

    if (!lvgl_port_lock(pdMS_TO_TICKS(100))) {
        ESP_LOGW(TAG, "Could not acquire LVGL lock, skipping weather update");
        return;
    }

    set_x_pos(170, 145, out_temp_l, (int)data.temperature);
    snprintf(weather_buffer, sizeof(weather_buffer), "%d",
             (int)data.temperature);
    lv_label_set_text(out_temp_l, weather_buffer);
    if (out_temp_l_ != NULL) {
        snprintf(weather_buffer, sizeof(weather_buffer), ".%d",
                 abs((int)(data.temperature * 10.0f) % 10));
        lv_label_set_text(out_temp_l_, weather_buffer);
    }

    set_x_pos(155, 130, out_hum_l, (int)data.humidity);
    snprintf(weather_buffer, sizeof(weather_buffer), "%d", data.humidity);
    lv_label_set_text(out_hum_l, weather_buffer);

    snprintf(weather_buffer, sizeof(weather_buffer), "%s", data.condition);
    lv_label_set_text(out_condition_l, weather_buffer);

    set_x_pos(155, 130, out_feels_like_l, (int)data.feels_like);
    snprintf(weather_buffer, sizeof(weather_buffer), "%d",
             (int)data.feels_like);
    lv_label_set_text(out_feels_like_l, weather_buffer);
    if (out_feels_like_l_ != NULL) {
        snprintf(weather_buffer, sizeof(weather_buffer), ".%d",
                 abs((int)(data.feels_like * 10.0f) % 10));
        lv_label_set_text(out_feels_like_l_, weather_buffer);
    }

    set_x_pos(155, 130, out_wind_l, (int)data.wind_speed);
    snprintf(weather_buffer, sizeof(weather_buffer), "%d",
             (int)data.wind_speed);
    lv_label_set_text(out_wind_l, weather_buffer);
    if (out_wind_l_ != NULL) {
        snprintf(weather_buffer, sizeof(weather_buffer), ".%d",
                 abs((int)(data.wind_speed * 10.0f) % 10));
        lv_label_set_text(out_wind_l_, weather_buffer);
    }

    lvgl_port_unlock();
    ESP_LOGI(TAG, "Weather labels updated");
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
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(500));
        if (xQueueReceive(queue, &data, pdMS_TO_TICKS(10000)) == pdTRUE) {
            update_weather_data(data);
        }
    }
}

// END OF SENSOR SCREEN

bool stage_wifi(void) {
    if (lvgl_port_lock(pdMS_TO_TICKS(100))) {
        lv_label_set_text(info_label,
                          "Initial connection\n- Disconnect from any "
                          "network\n- Connect to "
                          "esp_soft_ap\n- Search "
                          "in your browser:\nhttp:\\\\192.168.4.1\\");
        lvgl_port_unlock();
    }
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

    if (lvgl_port_lock(pdMS_TO_TICKS(100))) {
        lv_label_set_text(info_label, "Connecting to network");
        lvgl_port_unlock();
    }

    load_screen(screen_start);

    nvs_init();

    bool connected = wifi_start_auto();

    if (!connected) {
        stage_wifi();
    }

    create_sensor_screen();

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

    load_screen(screen_sensor);

    vTaskDelete(NULL);
}
