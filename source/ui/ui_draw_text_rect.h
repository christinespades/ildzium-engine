#pragma once
#include "core/fonts.h"
#include "ui/ui.h"
#include "ui/ui_params.h"

void draw_pixel(uint32_t* fb, int fb_w, int fb_h, int x, int y, uint32_t color);
void draw_rect(uint32_t* fb, int fb_w, int fb_h, int x, int y, int w, int h, uint32_t color);
void ui_draw_panel(uint32_t* fb, int fb_width, int fb_height,
                   int x, int y, int w, int h,
                   uint32_t bg_color, uint32_t border_color);
void draw_char(uint32_t* fb, int fb_w, int fb_h, char c, int x, int y, uint32_t color, int scale);
void draw_text(uint32_t* fb, int fb_w, int fb_h, const char* text, int x, int y, uint32_t color, int scale, UI_Button* b);
void draw_multiline_text(uint32_t* fb, int fb_w, int fb_h,
                         int x, int y, const char* text,
                         uint32_t color, int base_scale, int line_height, UI_Button* b);
void draw_text_clipped(uint32_t* fb, int fb_w, int fb_h,
                       int x, int y, const char* text,
                       uint32_t normal_color, int scale,
                       int clip_l, int clip_r, int clip_t, int clip_b,
                       UI_Button* b);