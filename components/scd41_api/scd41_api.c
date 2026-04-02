#include "scd41_api.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "scd41.h"
#include <string.h>

EventGroupHandle_t s_scd41_event_group = NULL;

static const char *TAG = "scd41_api";

static void init_i2c(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    ESP_ERROR_CHECK(i2c_param_config(TEST_I2C_PORT, &conf));
    ESP_ERROR_CHECK(
        i2c_driver_install(TEST_I2C_PORT, I2C_MODE_MASTER, 0, 0, 0));
}

void init_scd41(void) {
    init_i2c();

    scd41_config_t config = SCD41_CONFIG_DEFAULT();
    config.i2c_port = TEST_I2C_PORT;
    config.timeout_ms = 1000;

    ESP_ERROR_CHECK(scd41_init(&config));
    ESP_ERROR_CHECK(scd41_start_measurement());

    ESP_LOGI(TAG, "SCD41 started, waiting for first measurement...");
    vTaskDelay(pdMS_TO_TICKS(5000));
    xEventGroupSetBits(s_scd41_event_group, SCD41_READY);
}

esp_err_t read_scd41(scd41_custom_t *out) {
    scd41_data_t data;

    esp_err_t ret = scd41_read_measurement(&data);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read SCD41: %s", esp_err_to_name(ret));
        xEventGroupSetBits(s_scd41_event_group, SCD41_FALL_BIT);
        return ret;
    }

    out->co2 = data.co2_ppm;
    out->temperature = data.temperature;
    out->humidity = data.humidity;

    return ESP_OK;
}

void scd41_task(void *pvParameters) {
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    scd41_custom_t data;

    while (1) {
        if (read_scd41(&data) == ESP_OK) {
            xQueueOverwrite(queue, &data);
            ESP_LOGI(TAG, "CO2: %.1f | Temp: %.1f | Hum: %.1f", data.co2,
                     data.temperature, data.humidity);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
