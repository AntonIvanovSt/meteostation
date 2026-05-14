#include "buttons_api.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Configuration                                                       */
/* ------------------------------------------------------------------ */

#define BUTTON_1_GPIO GPIO_NUM_13
#define BUTTON_2_GPIO GPIO_NUM_14

/** Active-low wiring: button connects GPIO to GND, internal pull-up enabled. */
#define BUTTON_ACTIVE_LEVEL 0

/** Debounce window in milliseconds. */
#define DEBOUNCE_MS 20

#define TAG "buttons_api"
#define QUEUE_DEPTH 10

/* ------------------------------------------------------------------ */
/*  Internal types                                                      */
/* ------------------------------------------------------------------ */

typedef struct {
    button_id_t id;
    button_state_t raw_state; /* State sampled inside the ISR */
} isr_event_t;

/* ------------------------------------------------------------------ */
/*  Module-level state                                                  */
/* ------------------------------------------------------------------ */

static const gpio_num_t s_gpio[BUTTON_COUNT] = {
    [BUTTON_1] = BUTTON_1_GPIO,
    [BUTTON_2] = BUTTON_2_GPIO,
};

static button_state_t s_debounced[BUTTON_COUNT];
static button_event_cb_t s_user_cb = NULL;

static QueueHandle_t s_evt_queue = NULL;
static TaskHandle_t s_task = NULL;
static bool s_initialised = false;

/* ------------------------------------------------------------------ */
/*  ISR handler – runs in IRAM, must be fast                          */
/* ------------------------------------------------------------------ */

static void IRAM_ATTR gpio_isr_handler(void *arg) {
    button_id_t id = (button_id_t)(uintptr_t)arg;

    isr_event_t evt = {
        .id = id,
        .raw_state = (gpio_get_level(s_gpio[id]) == BUTTON_ACTIVE_LEVEL)
                         ? BUTTON_PRESSED
                         : BUTTON_RELEASED,
    };

    BaseType_t higher_woken = pdFALSE;
    xQueueSendFromISR(s_evt_queue, &evt, &higher_woken);
    portYIELD_FROM_ISR(higher_woken);
}

/* ------------------------------------------------------------------ */
/*  Debounce / event task                                              */
/* ------------------------------------------------------------------ */

static void button_task(void *arg) {
    isr_event_t evt;

    for (;;) {
        /* Block until an ISR fires. */
        if (xQueueReceive(s_evt_queue, &evt, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        /* Debounce: wait, then confirm the level is still the same. */
        vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));

        button_state_t confirmed =
            (gpio_get_level(s_gpio[evt.id]) == BUTTON_ACTIVE_LEVEL)
                ? BUTTON_PRESSED
                : BUTTON_RELEASED;

        if (confirmed == s_debounced[evt.id]) {
            /* Level did not change – bounce, ignore. */
            continue;
        }

        s_debounced[evt.id] = confirmed;

        ESP_LOGD(TAG, "Button %d %s", evt.id,
                 confirmed == BUTTON_PRESSED ? "PRESSED" : "RELEASED");

        if (s_user_cb) {
            button_event_t ev = (confirmed == BUTTON_PRESSED)
                                    ? BUTTON_EVENT_PRESSED
                                    : BUTTON_EVENT_RELEASED;
            s_user_cb(evt.id, ev);
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Public API                                                          */
/* ------------------------------------------------------------------ */

esp_err_t buttons_api_init(void) {
    if (s_initialised) {
        ESP_LOGW(TAG, "Already initialised");
        return ESP_OK;
    }

    /* Configure GPIO pins. */
    gpio_config_t io_cfg = {
        .pin_bit_mask = (1ULL << BUTTON_1_GPIO) | (1ULL << BUTTON_2_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };

    esp_err_t ret = gpio_config(&io_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "gpio_config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Sample initial states. */
    for (int i = 0; i < BUTTON_COUNT; i++) {
        s_debounced[i] = (gpio_get_level(s_gpio[i]) == BUTTON_ACTIVE_LEVEL)
                             ? BUTTON_PRESSED
                             : BUTTON_RELEASED;
    }

    /* Create the ISR event queue. */
    s_evt_queue = xQueueCreate(QUEUE_DEPTH, sizeof(isr_event_t));
    if (!s_evt_queue) {
        ESP_LOGE(TAG, "Failed to create event queue");
        return ESP_ERR_NO_MEM;
    }

    /* Install the shared GPIO ISR service (safe to call multiple times). */
    gpio_install_isr_service(0);

    /* Register per-pin ISR handlers. */
    for (int i = 0; i < BUTTON_COUNT; i++) {
        ret = gpio_isr_handler_add(s_gpio[i], gpio_isr_handler,
                                   (void *)(uintptr_t)i);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "gpio_isr_handler_add failed for pin %d: %s",
                     s_gpio[i], esp_err_to_name(ret));
            vQueueDelete(s_evt_queue);
            s_evt_queue = NULL;
            return ret;
        }
    }

    /* Start the debounce task. */
    BaseType_t created =
        xTaskCreate(button_task, "button_task", 2048, NULL, 5, &s_task);
    if (created != pdPASS) {
        ESP_LOGE(TAG, "Failed to create button task");
        for (int i = 0; i < BUTTON_COUNT; i++) {
            gpio_isr_handler_remove(s_gpio[i]);
        }
        vQueueDelete(s_evt_queue);
        s_evt_queue = NULL;
        return ESP_ERR_NO_MEM;
    }

    s_initialised = true;
    ESP_LOGI(TAG, "Initialised (GPIO %d, GPIO %d)", BUTTON_1_GPIO,
             BUTTON_2_GPIO);
    return ESP_OK;
}

esp_err_t buttons_api_deinit(void) {
    if (!s_initialised) {
        return ESP_OK;
    }

    for (int i = 0; i < BUTTON_COUNT; i++) {
        gpio_isr_handler_remove(s_gpio[i]);
    }

    if (s_task) {
        vTaskDelete(s_task);
        s_task = NULL;
    }

    if (s_evt_queue) {
        vQueueDelete(s_evt_queue);
        s_evt_queue = NULL;
    }

    s_user_cb = NULL;
    s_initialised = false;

    ESP_LOGI(TAG, "De-initialised");
    return ESP_OK;
}

void buttons_api_register_callback(button_event_cb_t cb) { s_user_cb = cb; }

esp_err_t buttons_api_get_state(button_id_t id, button_state_t *state) {
    if (id >= BUTTON_COUNT || !state) {
        return ESP_ERR_INVALID_ARG;
    }
    *state = s_debounced[id];
    return ESP_OK;
}

bool buttons_api_is_pressed(button_id_t id) {
    if (id >= BUTTON_COUNT) {
        return false;
    }
    return s_debounced[id] == BUTTON_PRESSED;
}
