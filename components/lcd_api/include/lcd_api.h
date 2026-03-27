#ifndef LCD_API_H_
#define LCD_API_H_

#define LCD_API_H_
#define LCD_HOST SPI2_HOST
#define LCD_PIXEL_CLK_HZ (40 * 1000 * 1000)
#define LCD_BK_LIGHT_ON 1
#define LCD_BK_LIGHT_OFF 0

#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 5
#define PIN_NUM_DC 16
#define PIN_NUM_RST 17
#define PIN_NUM_BK_LIGHT 4

// Display resolution
#define LCD_H_RES 240
#define LCD_V_RES 320

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

void init_lcd(void);

esp_lcd_panel_io_handle_t lcd_get_io_handle(void);
esp_lcd_panel_handle_t lcd_get_panel_handle(void);

#endif // !LCD_API_H_
