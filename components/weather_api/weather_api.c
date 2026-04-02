#include "weather_api.h"
#include "cJSON.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <string.h>

#define WEATHER_API_KEY "f0a4621d3e004a6ea1875957261303"
#define CITY "Tokyo"
#define WEATHER_API_URL                                                        \
    "http://api.weatherapi.com/v1/current.json?key=" WEATHER_API_KEY           \
    "&q=" CITY "&aqi=no"

#define MAX_HTTP_OUTPUT_BUFFER 2048
static char http_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
static int response_len = 0;
EventGroupHandle_t s_weather_event_group = NULL;

static const char *TAG = "weather_api";

static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key,
                 evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        if (response_len + evt->data_len < MAX_HTTP_OUTPUT_BUFFER) {
            memcpy(http_response_buffer + response_len, evt->data,
                   evt->data_len);
            response_len += evt->data_len;
            http_response_buffer[response_len] = 0;
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}

static esp_err_t parse_weather_response(const char *json_string,
                                        weather_data_t *out) {
    if (json_string == NULL || strlen(json_string) == 0) {
        ESP_LOGE(TAG, "Empty JSON response");
        return ESP_FAIL;
    }
    if (out == NULL) {
        ESP_LOGE(TAG, "Output struct is NULL");
        return ESP_FAIL;
    }

    cJSON *root = cJSON_Parse(json_string);
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGE(TAG, "JSON parse error before: %s", error_ptr);
        }
        return ESP_FAIL;
    }

    cJSON *current = cJSON_GetObjectItem(root, "current");
    if (current == NULL) {
        ESP_LOGE(TAG, "'current' object not found in JSON");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    cJSON *temp_c = cJSON_GetObjectItem(current, "temp_c");
    cJSON *feelslike_c = cJSON_GetObjectItem(current, "feelslike_c");
    cJSON *humidity = cJSON_GetObjectItem(current, "humidity");
    cJSON *wind_kph = cJSON_GetObjectItem(current, "wind_kph");
    cJSON *condition = cJSON_GetObjectItem(current, "condition");

    if (!cJSON_IsNumber(temp_c) || !cJSON_IsNumber(humidity)) {
        ESP_LOGE(TAG, "Required fields missing in JSON");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    out->temperature = (float)temp_c->valuedouble;
    out->feels_like = cJSON_IsNumber(feelslike_c)
                          ? (float)feelslike_c->valuedouble
                          : out->temperature;
    out->humidity = (int)humidity->valueint;
    out->wind_speed =
        cJSON_IsNumber(wind_kph) ? (float)wind_kph->valuedouble : 0.0f;

    const char *condition_text = "Unknown";
    if (condition != NULL) {
        cJSON *text = cJSON_GetObjectItem(condition, "text");
        if (cJSON_IsString(text) && text->valuestring != NULL) {
            condition_text = text->valuestring;
        }
    }
    strncpy(out->condition, condition_text, sizeof(out->condition) - 1);
    out->condition[sizeof(out->condition) - 1] = '\0';

    ESP_LOGI(TAG,
             "Weather: %.1f°C feels %.1f°C, humidity %d%%, wind %.1f kph, "
             "condition: %s",
             out->temperature, out->feels_like, out->humidity, out->wind_speed,
             out->condition);

    cJSON_Delete(root);
    return ESP_OK;
}

static esp_http_client_config_t init_weather(void) {
    esp_http_client_config_t config = {
        .url = WEATHER_API_URL,
        .event_handler = _http_event_handler,
        .timeout_ms = 10000,
    };
    return config;
}

static void reset_loop(esp_http_client_handle_t *client) {
    ESP_LOGE(TAG, "Trying again in 5s.");
    esp_http_client_cleanup(*client);
    xEventGroupSetBits(s_weather_event_group, WEATHER_FALL_BIT);
    vTaskDelay(pdMS_TO_TICKS(5000));
}

void weather_task(void *pvParameters) {
    QueueHandle_t weather_queue = (QueueHandle_t)pvParameters;
    esp_http_client_config_t config = init_weather();

    while (1) {
        response_len = 0;
        memset(http_response_buffer, 0, MAX_HTTP_OUTPUT_BUFFER);

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_err_t err = esp_http_client_perform(client);

        if (err == ESP_OK) {
            int status_code = esp_http_client_get_status_code(client);
            ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %lld",
                     status_code, esp_http_client_get_content_length(client));

            if (status_code == 200 && response_len > 0) {
                weather_data_t data = {0};
                if (parse_weather_response(http_response_buffer, &data) ==
                    ESP_OK) {
                    xQueueOverwrite(weather_queue, &data);
                    ESP_LOGI(TAG, "Weather data pushed to queue");
                    xEventGroupSetBits(s_weather_event_group, WEATHER_READY);
                } else {
                    ESP_LOGE(TAG, "Failed to parse weather data");
                    reset_loop(&client);
                    continue;
                }
            } else {
                ESP_LOGE(TAG, "HTTP request failed with status: %d",
                         status_code);
                reset_loop(&client);
                continue;
            }
        } else {
            ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
            reset_loop(&client);
            continue;
        }

        esp_http_client_cleanup(client);
        vTaskDelay(pdMS_TO_TICKS(60 * 60 * 1000));
    }
}
