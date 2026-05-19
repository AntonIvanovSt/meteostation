idf_component_register(SRCS "meteostation.c"
                            "../fonts/oswald_medium_48.c"
                            "../fonts/oswald_medium_20.c"
                            "../fonts/oswald_regular_20.c"
                    INCLUDE_DIRS "."
                    REQUIRES
                    ap_sta_api
                    lvgl_api
                    time_api
                    scd41_api
                    weather_api
)
