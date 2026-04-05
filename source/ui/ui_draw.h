#pragma once
#include "core/fonts.h"
#include "core/settings.h"
#include "ui/ui.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void draw_pixel(uint32_t* fb, int fb_w, int fb_h, int x, int y, uint32_t color);
static void draw_rect(uint32_t* fb, int fb_w, int fb_h, int x, int y, int w, int h, uint32_t color);
static void draw_char(uint32_t* fb, int fb_w, int fb_h, char c, int x, int y, uint32_t color, int scale);
void draw_text(uint32_t* fb, int fb_w, int fb_h, const char* text, int x, int y, uint32_t color, int scale);
void draw_multiline_text(uint32_t* fb, int fb_w, int fb_h,
                         int x, int y, const char* text,
                         uint32_t color, int base_scale, int line_height);
void draw_editor(UI_Button* b, uint32_t* fb, int fb_width, int fb_height, float dt);
void draw_scrollbar(UI_Button* b, uint32_t* fb, int fb_width, int fb_height);
void ui_draw(UI_Context* ctx, uint32_t* fb, int fb_width, int fb_height, float dt);
void draw_selection_highlighting(UI_Button* b, uint32_t* fb, int fb_width, int fb_height, int text_x, int draw_y);
void draw_line_numbers(UI_Button* b, uint32_t* fb, int fb_width, int fb_height, int text_x, int draw_y);