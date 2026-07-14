#include "pch.h"
#include "ui_draw_rect_panel.h"

void draw_rect(uint32_t* fb, int fb_w, int fb_h, int x, int y, int w, int h, uint32_t color)
{
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            draw_pixel(fb, fb_w, fb_h, x+i, y+j, color);
}

// Generic function to draw a panel with background and border
void draw_panel(uint32_t* fb, int fb_w, int fb_h, int x, int y, int w, int h, uint32_t bg_color, uint32_t border_color)
{
    // Background
    draw_rect(fb, fb_w, fb_h, x, y, w, h, bg_color);

    // Border (thickness = 2 pixels)
    draw_rect(fb, fb_w, fb_h, x, y, w, 2, border_color);                    // top
    draw_rect(fb, fb_w, fb_h, x, y + h - 2, w, 2, border_color);            // bottom
    draw_rect(fb, fb_w, fb_h, x, y, 2, h, border_color);                    // left
    draw_rect(fb, fb_w, fb_h, x + w - 2, y, 2, h, border_color);            // right
}
