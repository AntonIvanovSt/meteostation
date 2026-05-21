#include "button_api.h"
#include "ap_sta_api.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include <stdio.h>

// GPIO config
#define ON_OFF_BUTTON GPIO_NUM_32
#define DEBOUNCE_TIME_MS 50
#define PIN_NUM_BK_LIGHT 4

extern EventGroupHandle_t s_wifi_event_group;

static const char *TAG = "button_api";

void on_off_button_task(void *arg) {
    // Configure GPIO as input
    gpio_config_t io_conf = {.pin_bit_mask = (1ULL << ON_OFF_BUTTON),
                             .mode = GPIO_MODE_INPUT,
                             .pull_up_en =
                                 GPIO_PULLUP_ENABLE, // Enable internal pull-up
                             .pull_down_en = GPIO_PULLDOWN_DISABLE,
                             .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);

    gpio_reset_pin(PIN_NUM_BK_LIGHT);
    gpio_set_direction(PIN_NUM_BK_LIGHT, GPIO_MODE_OUTPUT);

    ESP_LOGI(TAG, "Button monitoring started on GPIO %d", ON_OFF_BUTTON);

    bool last_state = true;
    bool current_state;
    bool screen_state = true;

    while (1) {
        current_state = gpio_get_level(ON_OFF_BUTTON);

        if (last_state && !current_state) {
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME_MS));
            current_state = gpio_get_level(ON_OFF_BUTTON);

            if (!current_state) {
                ESP_LOGI(TAG, "Power button PRESSED");

                EventBits_t uxBits = 0;
                if (s_wifi_event_group != NULL) {
                    uxBits = xEventGroupGetBits(s_wifi_event_group);
                }

                if (uxBits & WIFI_CONNECTED_BIT) {
                    screen_state = !screen_state;
                    gpio_set_level(PIN_NUM_BK_LIGHT, screen_state ? 1 : 0);
                    ESP_LOGI(TAG, "Backlight toggled: %s",
                             screen_state ? "ON" : "OFF");
                } else {
                    ESP_LOGI(
                        TAG,
                        "WiFi not connected yet. Signaling boot proceed bit.");
                    if (s_wifi_event_group != NULL) {
                        xEventGroupSetBits(s_wifi_event_group,
                                           BUTTON_PROCEED_BIT);
                    }
                }
            }
        }

        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
