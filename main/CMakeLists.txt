idf_component_register(SRCS "meteostation.c"
                            "../fonts/kode_mono_24.c"
                    INCLUDE_DIRS "."
                    REQUIRES lvgl_api
                    scd41_api
                    time_api
                    weather_api
                    wifi_connect
                    )
