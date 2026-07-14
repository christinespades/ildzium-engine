#pragma once
#include "core/fonts.h"
#include "ui/ui.h"
#include "core/params/params.h"
#include "ui/ui_text.h"
#include "ui/ui_draw.h"
#include "ui/ui_draw_rect_panel.h"

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