#include "lvgl_api.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "lcd_api.h"
#include "lv_api_map_v8.h"
#include <stdbool.h>

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

lv_obj_t *create_background(lv_color_t color) {
    if (lvgl_port_lock(0)) {
        lv_obj_t *screen = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(screen, color, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
        lvgl_port_unlock();
        return screen;
    }
    return NULL;
}

lv_obj_t *create_label(lv_obj_t *display_label, const lv_font_t *font,
                       lv_color_t color, int x, int y, const char *text) {
    if (lvgl_port_lock(0)) {

        lv_obj_t *value_label = lv_label_create(display_label);
        lv_label_set_text(value_label, text);

        lv_obj_set_style_text_font(value_label, font, 0);

        lv_obj_set_style_text_color(value_label, color, 0);
        lv_obj_set_style_text_opa(value_label, LV_OPA_COVER, 0);

        lv_obj_set_pos(value_label, x, y);
        lvgl_port_unlock();
        return value_label;
    }
    return NULL;
}

lv_obj_t *create_image(lv_obj_t *parent, const lv_img_dsc_t *img, int x,
                       int y) {
    if (lvgl_port_lock(0)) {
        lv_obj_t *img_obj = lv_img_create(parent);
        lv_img_set_src(img_obj, img);
        lv_obj_set_pos(img_obj, x, y);
        lvgl_port_unlock();
        return img_obj;
    }
    return NULL;
}

// in lv_color_make order is BRG with RGB565 notation
// max values are: 31, 31, 63

lv_obj_t *create_line(lv_point_precise_t *line_array, lv_obj_t *display_label,
                      lv_color_t color, int x, int y) {
    if (lvgl_port_lock(0)) {
        lv_obj_t *line = lv_line_create(display_label);
        lv_line_set_points(line, line_array, 2);

        // Position the line
        lv_obj_set_pos(line, x, y);

        // Style the line
        lv_obj_set_style_line_width(line, 2, 0);
        lv_obj_set_style_line_color(line, color, 0);
        lvgl_port_unlock();
        return line;
    }
    return NULL;
}
