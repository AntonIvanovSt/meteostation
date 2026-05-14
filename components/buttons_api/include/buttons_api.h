#pragma once

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Button identifiers
 */
typedef enum {
    BUTTON_1 = 0, /*!< Button on GPIO 32 */
    BUTTON_2,     /*!< Button on GPIO 34 */
    BUTTON_COUNT
} button_id_t;

/**
 * @brief Button state
 */
typedef enum { BUTTON_RELEASED = 0, BUTTON_PRESSED } button_state_t;

/**
 * @brief Button event types
 */
typedef enum { BUTTON_EVENT_PRESSED = 0, BUTTON_EVENT_RELEASED } button_event_t;

/**
 * @brief Button event callback signature
 *
 * @param id    Which button triggered the event
 * @param event BUTTON_EVENT_PRESSED or BUTTON_EVENT_RELEASED
 */
typedef void (*button_event_cb_t)(button_id_t id, button_event_t event);

/**
 * @brief Initialise the buttons_api component.
 *        Configures GPIO 32 and GPIO 34 as inputs with internal pull-ups and
 *        installs an ISR-based debounce task.
 *
 * @return ESP_OK on success, or an esp_err_t error code.
 */
esp_err_t buttons_api_init(void);

/**
 * @brief De-initialise the component and release all resources.
 *
 * @return ESP_OK on success.
 */
esp_err_t buttons_api_deinit(void);

/**
 * @brief Register a callback invoked on every press / release event.
 *        Pass NULL to unregister.
 *
 * @param cb  Callback function pointer (called from a FreeRTOS task context).
 */
void buttons_api_register_callback(button_event_cb_t cb);

/**
 * @brief Read the instantaneous (debounced) state of a button.
 *
 * @param id     Button to query.
 * @param[out] state  Filled with BUTTON_PRESSED or BUTTON_RELEASED.
 * @return ESP_OK, or ESP_ERR_INVALID_ARG for an unknown id.
 */
esp_err_t buttons_api_get_state(button_id_t id, button_state_t *state);

/**
 * @brief Check whether a button is currently pressed (convenience wrapper).
 *
 * @param id  Button to query.
 * @return true if pressed, false otherwise.
 */
bool buttons_api_is_pressed(button_id_t id);

#ifdef __cplusplus
}
#endif
