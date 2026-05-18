idf_component_register(SRCS "ap_sta_api.c"
                    INCLUDE_DIRS "include"
                    REQUIRES
                    nvs_flash
                    esp_wifi
                    esp_http_server)
