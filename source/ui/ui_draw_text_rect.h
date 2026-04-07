#pragma once
#include <stdint.h>

void draw_pixel(uint32_t* fb, int fb_w, int fb_h, int x, int y, uint32_t color);
void draw_rect(uint32_t* fb, int fb_w, int fb_h, int x, int y, int w, int h, uint32_t color);
void ui_draw_panel(uint32_t* fb, int fb_width, int fb_height,
                   int x, int y, int w, int h,
                   uint32_t bg_color, uint32_t border_color);
void draw_char(uint32_t* fb, int fb_w, int fb_h, char c, int x, int y, uint32_t color, int scale);
void draw_text(uint32_t* fb, int fb_w, int fb_h, const char* text, int x, int y, uint32_t color, int scale);
void draw_multiline_text(uint32_t* fb, int fb_w, int fb_h,
                         int x, int y, const char* text,
                         uint32_t color, int base_scale, int line_height);