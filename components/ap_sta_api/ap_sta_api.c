#include "ap_sta_api.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_base.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_wifi_types_generic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ESP_WIFI_SSID "esp_soft_ap"
#define ESP_WIFI_PASS "password"

#define NVS_WIFI_NAMESPACE "wifi_creds"
#define NVS_KEY_SSID "ssid"
#define NVS_KEY_PASS "pass"

#define ESP_WIFI_CHANNEL 1
#define ESP_MAX_STA_CONN 2
#define ESP_MAX_RETRY 5
static const char *TAG = "wifi_api";

static int s_retry_num = 0;
static esp_netif_t *s_ap_netif = NULL;
static esp_netif_t *s_sta_netif = NULL;
EventGroupHandle_t s_wifi_event_group = NULL;

// NVS helper functions
static void save_wifi_credentials(const char *ssid, const char *pass) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_WIFI_NAMESPACE, NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        nvs_set_str(handle, NVS_KEY_SSID, ssid);
        nvs_set_str(handle, NVS_KEY_PASS, pass);
        nvs_commit(handle);
        nvs_close(handle);
        ESP_LOGI(TAG, "Credentials saved to NVS.");
    } else {
        ESP_LOGE(TAG, "Failed to open NVS to save credentials: %s",
                 esp_err_to_name(err));
    }
}

static bool load_wifi_credentials(char *ssid, size_t ssid_len, char *pass,
                                  size_t pass_len) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_WIFI_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return false;
    }

    esp_err_t err_ssid = nvs_get_str(handle, NVS_KEY_SSID, ssid, &ssid_len);
    esp_err_t err_pass = nvs_get_str(handle, NVS_KEY_PASS, pass, &pass_len);
    nvs_close(handle);

    return (err_ssid == ESP_OK && err_pass == ESP_OK);
}

// ─── Event Handler
// ────────────────────────────────────────────────────────────

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {

    // AP events
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *e = event_data;
        ESP_LOGI(TAG, "Station " MACSTR " joined, AID=%d", MAC2STR(e->mac),
                 e->aid);

    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *e = event_data;
        ESP_LOGI(TAG, "Station " MACSTR " left, AID=%d", MAC2STR(e->mac),
                 e->aid);

        // STA events
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "STA started, connecting...");

    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGW(TAG, "Retrying connection (%d/%d)...", s_retry_num,
                     ESP_MAX_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Connection failed after %d retries", ESP_MAX_RETRY);
        }

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *e = event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&e->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// Initialize baseline driver configs (run once regardless of AP or STA mode)
static void wifi_init_base(void) {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));
}

// Automatically decides whether to load stored credentials or open captive
// portal
bool wifi_start_auto(void) {
    char stored_ssid[64] = {0};
    char stored_pass[64] = {0};

    wifi_init_base();

    if (load_wifi_credentials(stored_ssid, sizeof(stored_ssid), stored_pass,
                              sizeof(stored_pass))) {
        ESP_LOGI(TAG, "Found stored credentials for SSID: %s. Connecting...",
                 stored_ssid);

        s_sta_netif = esp_netif_create_default_wifi_sta();

        wifi_config_t sta_config = {0};
        strncpy((char *)sta_config.sta.ssid, stored_ssid,
                sizeof(sta_config.sta.ssid) - 1);
        strncpy((char *)sta_config.sta.password, stored_pass,
                sizeof(sta_config.sta.password) - 1);

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
        s_retry_num = 0;
        ESP_ERROR_CHECK(esp_wifi_start());

        EventBits_t bits = xEventGroupWaitBits(
            s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE,
            pdFALSE, pdMS_TO_TICKS(15000));

        if (bits & WIFI_CONNECTED_BIT) {
            return true;
        }

        ESP_LOGW(TAG,
                 "Stored credentials failed. Falling back to SoftAP mode.");
        esp_wifi_stop();
        if (s_sta_netif) {
            esp_netif_destroy(s_sta_netif);
            s_sta_netif = NULL;
        }
    }

    // Fallback: No credentials found OR stored credentials failed to connect.
    ESP_LOGI(TAG, "Starting provision configuration portal...");
    s_ap_netif = esp_netif_create_default_wifi_ap();

    wifi_config_t ap_config = {
        .ap =
            {
                .ssid = ESP_WIFI_SSID,
                .ssid_len = strlen(ESP_WIFI_SSID),
                .channel = ESP_WIFI_CHANNEL,
                .password = ESP_WIFI_PASS,
                .max_connection = ESP_MAX_STA_CONN,
                .authmode = WIFI_AUTH_WPA2_PSK,
                .pmf_cfg = {.required = true},
            },
    };
    if (strlen(ESP_WIFI_PASS) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    start_webserver();
    return false; // Did not connect yet, portal active
}

// nvs init
void nvs_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
}

// ─── Switch to STA ───────────────────────────────────────────────────────────

static void wifi_switch_to_sta(const char *ssid, const char *pass) {
    ESP_LOGI(TAG, "Switching to STA mode. SSID:%s", ssid);

    // Stop AP
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Destroy old AP netif, create STA netif
    if (s_ap_netif) {
        esp_netif_destroy(s_ap_netif);
        s_ap_netif = NULL;
    }
    s_sta_netif = esp_netif_create_default_wifi_sta();

    // Configure STA credentials
    wifi_config_t sta_config = {0};
    strncpy((char *)sta_config.sta.ssid, ssid, sizeof(sta_config.sta.ssid) - 1);
    strncpy((char *)sta_config.sta.password, pass,
            sizeof(sta_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));

    s_retry_num = 0;
    ESP_ERROR_CHECK(
        esp_wifi_start()); // triggers WIFI_EVENT_STA_START → connect
}

// ─── HTTP Handlers ───────────────────────────────────────────────────────────

static esp_err_t form_get_handler(httpd_req_t *req) {
    const char *html =
        "<!DOCTYPE html><html><body>"
        "<h1>WiFi Configuration</h1>"
        "<form action=\"/submit\" method=\"POST\">"
        "  <label>SSID: <input type=\"text\" name=\"ssid\"></label><br><br>"
        "  <label>PASS: <input type=\"password\" name=\"pass\"></label><br><br>"
        "  <button type=\"submit\">Connect</button>"
        "</form>"
        "</body></html>";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static void wifi_switch_task(void *param) {
    char *args = (char *)param;
    vTaskDelay(pdMS_TO_TICKS(500)); // HTTP response flush

    char ssid[64] = {0}, pass[64] = {0};
    char *sep = strchr(args, '|');
    if (sep) {
        strncpy(ssid, args, sep - args);
        strncpy(pass, sep + 1, sizeof(pass) - 1);
    }
    free(args);

    save_wifi_credentials(ssid, pass);
    wifi_switch_to_sta(ssid, pass);

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE,
        pdFALSE, pdMS_TO_TICKS(15000));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Successfully connected to WiFi!");
    } else {
        ESP_LOGE(TAG, "Failed to connect to WiFi.");
    }

    vTaskDelete(NULL);
}

static esp_err_t form_post_handler(httpd_req_t *req) {
    char body[256] = {0};
    int total_len = req->content_len;

    if (total_len >= (int)sizeof(body)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Body too long");
        return ESP_FAIL;
    }

    int received = httpd_req_recv(req, body, total_len);
    if (received <= 0) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Receive failed");
        return ESP_FAIL;
    }
    body[received] = '\0';
    ESP_LOGI(TAG, "Raw POST body: %s", body);

    char ssid_val[64] = {0};
    char pass_val[64] = {0};

    httpd_query_key_value(body, "ssid", ssid_val, sizeof(ssid_val));
    httpd_query_key_value(body, "pass", pass_val, sizeof(pass_val));

    ESP_LOGI(TAG, "Received — SSID: %s | PASS: %s", ssid_val, pass_val);

    // Send response BEFORE switching WiFi (connection will drop)
    const char *resp = "<!DOCTYPE html><html><body>"
                       "<h1>Connecting...</h1>"
                       "<p>ESP32 is now connecting to your WiFi.</p>"
                       "<p>This access point will disappear. Check your router "
                       "for the ESP32 IP.</p>"
                       "</body></html>";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    // Schedule the switch in a separate task so the HTTP response finishes
    // first
    char *args = malloc(128);
    snprintf(args, 128, "%s|%s", ssid_val, pass_val);

    xTaskCreate(wifi_switch_task, "wifi_switch_task", 4096, args, 5, NULL);

    return ESP_OK;
}

// ─── URI & Server
// ─────────────────────────────────────────────────────────────

static const httpd_uri_t uri_get = {.uri = "/",
                                    .method = HTTP_GET,
                                    .handler = form_get_handler,
                                    .user_ctx = NULL};
static const httpd_uri_t uri_post = {.uri = "/submit",
                                     .method = HTTP_POST,
                                     .handler = form_post_handler,
                                     .user_ctx = NULL};

httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
        ESP_LOGI(TAG, "HTTP server started");
        return server;
    }
    ESP_LOGE(TAG, "Failed to start HTTP server");
    return NULL;
}
