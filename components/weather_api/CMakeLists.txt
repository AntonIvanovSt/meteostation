idf_component_register(SRCS "weather_api.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_client esp_http_server json esp_timer)
