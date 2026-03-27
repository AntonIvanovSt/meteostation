#ifndef LVGL_API_H_
#define LVGL_API_H_
// Display rotation
#define LV_DISP_ROT_NONE 0
#define LV_DISP_ROT_90 1
#define LV_DISP_ROT_180 2
#define LV_DISP_ROT_270 3

// LVGL settings
#define LVGL_TICK_PERIOD_MS 2
#define LVGL_BUFFER_HEIGHT 50

// LVGL colors
#define COLOR_DARK_PURPLE lv_color_make(30, 15, 39)
#define COLOR_WHITE lv_color_make(31, 31, 63)
#define COLOR_ORANGE lv_color_make(0, 27, 35)
#define COLOR_BLACK lv_color_make(0, 0, 0)
#define COLOR_PINK lv_color_make(15, 31, 30)
#define COLOR_GREEN lv_color_make(7, 7, 30)
#define COLOR_CYAN lv_color_make(31, 0, 63)

void init_lvgl(int rotation);

#endif // !LVGL_API_H_
