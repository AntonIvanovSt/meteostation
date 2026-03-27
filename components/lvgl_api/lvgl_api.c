#include "lvgl_api.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "lcd_api.h"
#include "lvgl.h"

static const char *TAG = "lvgl_api";

static lv_disp_t *lvgl_disp = NULL;

void init_lvgl(int rotation) {
    init_lcd();
    ESP_LOGI(TAG, "Initialize LVGL");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_get_io_handle(),
        .panel_handle = lcd_get_panel_handle(),
        .buffer_size = LCD_H_RES * LVGL_BUFFER_HEIGHT * sizeof(uint16_t),
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .rotation =
            {
                .swap_xy = false,
                .mirror_x = false,
                .mirror_y = false,
            },
        .flags = {
            .buff_dma = true,
        }};
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);

    // Rotate display to portrait mode if needed
    lv_disp_set_rotation(lvgl_disp, rotation); // 0 - no rotation

    ESP_LOGI(TAG, "Setup complete");
}
