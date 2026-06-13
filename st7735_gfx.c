#include <stdio.h>
#include "st7735_gfx.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

#define TAG "GFX"

// |slope| > 1
static void LineDrawHighSlope(esp_lcd_panel_handle_t panel_handle, uint16_t color, int x_start, int y_start, int x_end, int y_end)
{
    int x, y;
    int dx = abs(x_end - x_start);
    int dy = abs(y_end - y_start);
    int diff =  2 * dx - dy;
    int x_inc = 1;
    int y_inc = 1;
    x = x_start;
    y = y_start;

    if (x_end - x_start < 0)
    {
        x_inc = -1;
    }

    if (y_end - y_start < 0)
    {
        y_inc = -1;
    }

    while(y != y_end)
    {
        ESP_LOGD("LCD","Drawing: P(%d, %d)", x, y);
        esp_lcd_panel_draw_bitmap(panel_handle, x , y, x + 1, y + 1, &color);
        if(diff > 0)
        {
            diff = diff + (2 * (dx - dy));
            x += x_inc;
        }
        else
        {
            diff = diff + 2 * dx;
        }
        y += y_inc;
    }
}

// slopes between 0 and -1 or 0 and 1
static void LineDrawLowSlope(esp_lcd_panel_handle_t panel_handle, uint16_t color, int x_start, int y_start, int x_end, int y_end)
{
    int x, y;
    int dx = abs(x_end - x_start);
    int dy = abs(y_end - y_start);
    int diff =  2 * dy - dx;
    int x_inc = 1;
    int y_inc = 1;
    x = x_start;
    y = y_start;

    if(y_end - y_start < 0)
    {
        y_inc = -1;
    }

    if (x_end - x_start < 0)
    {
        x_inc = -1;
    }

    while(x != x_end)
    {
        ESP_LOGD("LCD","Drawing: P(%d, %d)", x, y);
        esp_lcd_panel_draw_bitmap(panel_handle, x , y, x + 1, y + 1, &color);
        if(diff > 0)
        {
            diff = diff + (2 * (dy - dx));
            y += y_inc;
        }
        else
        {
            diff = diff + 2 * dy;
        }
        x += x_inc;
    }
}

static void LineDrawHorizontal(esp_lcd_panel_handle_t panel_handle, uint16_t color, int x_start, int y_start, int x_end, int y_end)
{
    int length = abs(x_end - x_start);
    
    if (length == 0)
    {
        esp_lcd_panel_draw_bitmap(panel_handle, x_start, y_start, x_start + 1, y_start + 1, &color);
        return;
    }

    uint8_t *color_buf = (uint8_t*)heap_caps_malloc(length * 2, MALLOC_CAP_DMA);
    if (color_buf == NULL)
    {
        ESP_LOGE("LCD", "malloc failed");
        return;
    }

    for (uint32_t i = 0; i < length; i++)
    {
        color_buf[i * 2] = (color >> 8) & 0xFF;;
        color_buf[i * 2 + 1] = color & 0xFF;
    }
    if(x_start < x_end)
    {
        esp_lcd_panel_draw_bitmap(panel_handle, x_start , y_start, x_end, y_end + 1, (uint16_t *)color_buf);
    }
    else
    {
        esp_lcd_panel_draw_bitmap(panel_handle, x_end, y_end, x_start , y_start + 1, (uint16_t *)color_buf);
    }
    free(color_buf);
}

static void LineDrawVertical(esp_lcd_panel_handle_t panel_handle, uint16_t color, int x_start, int y_start, int x_end, int y_end)
{
    int length = abs(y_end - y_start);
    
    uint8_t *color_buf = (uint8_t*)heap_caps_malloc(length * 2, MALLOC_CAP_DMA);
    if (color_buf == NULL)
    {
        ESP_LOGE("LCD", "malloc failed");
        return;
    }

    for (uint32_t i = 0; i < length; i++)
    {
        color_buf[i * 2] = (color >> 8) & 0xFF;;
        color_buf[i * 2 + 1] = color & 0xFF;
    }

    if(y_start < y_end)
    {
        esp_lcd_panel_draw_bitmap(panel_handle, x_start , y_start, x_end + 1, y_end, (uint16_t *)color_buf);
    }
    else
    {
        esp_lcd_panel_draw_bitmap(panel_handle, x_end, y_end, x_start + 1 , y_start, (uint16_t *)color_buf);
    }
    free(color_buf);
}

static void RectangleDrawSolid(esp_lcd_panel_handle_t panel_handle, uint16_t color, int x_start, int y_start, int x_end, int y_end)
{
    int length = abs(x_end - x_start);
    int height = abs(y_end - y_start);

    uint32_t pixel_count = length * height;
    uint8_t *color_buf = (uint8_t*)heap_caps_malloc(pixel_count * 2, MALLOC_CAP_DMA);
    if (color_buf == NULL)
    {
        ESP_LOGE("LCD", "malloc failed");
        return;
    }

    for (uint32_t i = 0; i < pixel_count; i++)
    {
        color_buf[i * 2] = (color >> 8) & 0xFF;;
        color_buf[i * 2 + 1] = color & 0xFF;
    }
    if(x_start < x_end)
    {
        esp_lcd_panel_draw_bitmap(panel_handle, x_start, y_start, x_end, y_end, (uint16_t *)color_buf);
    }
    else
    {
        esp_lcd_panel_draw_bitmap(panel_handle, x_end, y_end, x_start, y_start, (uint16_t *)color_buf);
    }
    free(color_buf);
}

static void RectangleDrawFrame(esp_lcd_panel_handle_t panel_handle, uint16_t color, int x_start, int y_start, int x_end, int y_end)
{
    LineDrawVertical(panel_handle, color, x_start, y_start, x_start, y_end);   // Left: |
    LineDrawVertical(panel_handle, color, x_end, y_start, x_end, y_end);       // Right: |
    LineDrawHorizontal(panel_handle, color, x_start, y_start, x_end, y_start); // Top: -
    LineDrawHorizontal(panel_handle, color, x_start, y_end, x_end, y_end);     // Bottom: _
}

static void TriangleDrawFrame(esp_lcd_panel_handle_t panel_handle, uint16_t color, int ax, int ay, int bx, int by, int cx, int cy)
{
    LineDraw(panel_handle, color,  ax, ay, bx, by);
    LineDraw(panel_handle, color, ax, ay, cx, cy);
    LineDraw(panel_handle, color, cx, cy, bx, by);
}

static void TriangleDrawSolid(esp_lcd_panel_handle_t panel_handle, uint16_t color, int ax, int ay, int bx, int by, int cx, int cy)
{
    
    int x_temp, y_temp;
    if (ay < by) 
    { 
        x_temp=ax; y_temp=ay; 
        ax=bx; ay=by; 
        bx=x_temp; by=y_temp; 
    }
    if (ay < cy) 
    { 
        x_temp=ax; y_temp=ay; 
        ax=cx; ay=cy; 
        cx=x_temp; cy=y_temp; 
    }
    if (by < cy) 
    { 
        x_temp=bx; y_temp=by; 
        bx=cx; by=cy; 
        cx=x_temp; cy=y_temp; 
    }
    
    int y = ay;
    while(y > cy)
    {
        int x_right;
        int x_left;
        if(y >= by)
        {
            x_right = ax + ((y - ay) * (ax - bx) + (ay - by) / 2) / (ay - by);
        }
        else
        {
            x_right = bx + ((y - by) * (bx - cx) + (by - cy) / 2) / (by - cy);
        }
        x_left = ax + (y - ay) * (ax - cx) / (ay - cy);
        LineDrawHorizontal(panel_handle, color, x_left, y, x_right + 1, y);
        y--;
    }
}

static void CircleDrawFrame(esp_lcd_panel_handle_t panel_handle, uint16_t color, int xc, int yc, int (*pixel_arr)[2], int pixel_count)
{
    for(int i = 0; i < pixel_count; i++)
    {
        int x = pixel_arr[i][0];
        int y = pixel_arr[i][1];

        // first quadrant (+x, +y)
        esp_lcd_panel_draw_bitmap(panel_handle, xc + x, yc + y, xc + x + 1, yc + y + 1, &color);
        esp_lcd_panel_draw_bitmap(panel_handle, xc + y, yc + x, xc + y + 1, yc + x + 1, &color);

        // second quadrant (-x, +y)
        esp_lcd_panel_draw_bitmap(panel_handle, xc - x, yc + y, xc - x + 1, yc + y + 1, &color);
        esp_lcd_panel_draw_bitmap(panel_handle, xc - y, yc + x, xc - y + 1, yc + x + 1, &color);
        
        // third quadrant (-x, -y)
        esp_lcd_panel_draw_bitmap(panel_handle, xc - x, yc - y, xc - x + 1, yc - y + 1, &color);
        esp_lcd_panel_draw_bitmap(panel_handle, xc - y, yc - x, xc - y + 1, yc - x + 1, &color);

        // fourth quadrant (+x, -y)
        esp_lcd_panel_draw_bitmap(panel_handle, xc + x, yc - y, xc + x + 1, yc - y + 1, &color);
        esp_lcd_panel_draw_bitmap(panel_handle, xc + y, yc - x, xc + y + 1, yc - x + 1, &color);
    }
}

static void CircleDrawSolid(esp_lcd_panel_handle_t panel_handle, uint16_t color, int xc, int yc, int (*pixel_arr)[2], int pixel_count)
{
    for(int j = 0; j < pixel_count; j++)
    {
        int x = pixel_arr[j][0];
        int y = pixel_arr[j][1];

        int x_left = xc - x;
        int x_right = xc + x;
        int yh = yc + y;
        LineDrawHorizontal(panel_handle, color, x_left, yh, x_right, yh);
        yh = yc - y;
        LineDrawHorizontal(panel_handle, color, x_left, yh, x_right, yh);
        
        x_left = xc - y;
        x_right = xc + y;
        yh = yc + x;
        LineDrawHorizontal(panel_handle, color, x_left, yh, x_right, yh);
        yh = yc - x;
        LineDrawHorizontal(panel_handle, color, x_left, yh, x_right, yh);
    }
}

static int CalculateFirstOctantPixels(int r, int (*pixel_arr)[2])
{
    int x = 0;
    int y = r;
    int d = 3 - (2 * r);
    int count = 0;
    while(x <= y)
    {
        pixel_arr[x][0] = x;
        pixel_arr[x][1] = y;
        if(d < 0)
        {
            d = d + (4 * x) + 6;
        }
        else
        {
            d = d + (4 * (x - y) ) + 10;
            y--;
        }
        x++;
        count++;
    }
    return count;
}

// Public functions
void LineDraw(esp_lcd_panel_handle_t panel_handle, uint16_t color, int x_start, int y_start, int x_end, int y_end)
{
    ESP_LOGI(TAG, "Drawing line from P1(%d, %d) to P2(%d, %d)", x_start, y_start, x_end, y_end);
    if(y_end - y_start == 0)
    {
        ESP_LOGI(TAG, "Drawing Horizontal Line");
        LineDrawHorizontal(panel_handle, color, x_start, y_start, x_end, y_end);
        return;
    }
    else if(x_end - x_start  == 0)
    {
        ESP_LOGI(TAG, "Drawing Vertical Line");
        LineDrawVertical(panel_handle, color, x_start, y_start, x_end, y_end);
        return;
    }

    if(x_start < x_end)
    {
        if(abs(x_end - x_start) >= abs(y_end - y_start))
        {
            LineDrawLowSlope(panel_handle, color, x_start, y_start, x_end, y_end);
            return;
        }
        else
        {
            LineDrawHighSlope(panel_handle, color, x_start, y_start, x_end, y_end);
            return;
        }
    }
    else
    {
        if(abs(x_start - x_end) >= abs(y_end - y_start))
        {
            LineDrawLowSlope(panel_handle, color, x_end, y_end, x_start, y_start);
            return;
        }
        else
        {
            LineDrawHighSlope(panel_handle, color, x_end, y_end, x_start, y_start);
            return;
        }
    }
}

void RectangleDraw(esp_lcd_panel_handle_t panel_handle, uint16_t color, bool solid, int x_start, int y_start, int x_end, int y_end)
{
    if(solid)
    {
        RectangleDrawSolid(panel_handle, color, x_start, y_start, x_end, y_end);
    }
    else
    {
        RectangleDrawFrame(panel_handle, color, x_start, y_start, x_end, y_end);
    }
}

void TriangleDraw(esp_lcd_panel_handle_t panel_handle, uint16_t color, bool solid, int ax, int ay, int bx, int by, int cx, int cy)
{
    if(solid)
    {
        TriangleDrawSolid(panel_handle, color, ax, ay, bx, by, cx, cy);
    }
    else
    {
        TriangleDrawFrame(panel_handle, color, ax, ay, bx, by, cx, cy);
    }
}

void CircleDraw(esp_lcd_panel_handle_t panel_handle, uint16_t color, bool solid, int xc, int yc, int r)
{
    int (* pixel_arr)[2] = (int (*)[2])heap_caps_malloc((r + 1) * sizeof(*pixel_arr), MALLOC_CAP_DMA);
    int pixel_count = CalculateFirstOctantPixels(r, pixel_arr); 
    if( pixel_count == 0)
    {
        ESP_LOGE("LCD","Pixel count invalid: %d", pixel_count);
        goto exit;
    }

    if(solid)
    {
        CircleDrawSolid(panel_handle, color, xc, yc, pixel_arr, pixel_count);
    }
    else
    {
        CircleDrawFrame(panel_handle, color, xc, yc, pixel_arr, pixel_count);
    }
exit:
    free(pixel_arr);
}
