#ifndef INCLUDE_ST7735_GFX
#define INCLUDE_ST7735_GFX

#include <stdint.h>
#include <stdbool.h>
#include "esp_lcd_panel_ops.h"

void LineDraw(esp_lcd_panel_handle_t panel_handle, uint16_t color, int x_start, int y_start, int x_end, int y_end);
void RectangleDraw(esp_lcd_panel_handle_t panel_handle, uint16_t color, bool solid, int x_start, int y_start, int x_end, int y_end);
void TriangleDraw(esp_lcd_panel_handle_t panel_handle, uint16_t color, bool solid, int ax, int ay, int bx, int by, int cx, int cy);
void CircleDraw(esp_lcd_panel_handle_t panel_handle, uint16_t color, bool solid, int xc, int yc, int r);

#endif // INCLUDE_ST7735_GFX