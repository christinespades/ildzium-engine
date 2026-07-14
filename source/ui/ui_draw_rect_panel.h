#pragma once
#include "ui/ui_draw_pixel.h"

void draw_rect(uint32_t* fb, int fb_w, int fb_h, int x, int y, int w, int h, uint32_t color);
void draw_panel(uint32_t* fb, int fb_w, int fb_h, int x, int y, int w, int h,uint32_t bg_color, uint32_t border_color);