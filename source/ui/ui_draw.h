#pragma once
#include "core/debug.h"
#include "core/fonts.h"
#include "core/settings.h"
#include "ui/ui.h"
#include "ui/ui_borders.h"
#include "ui/ui_callbacks.h"
#include "ui/ui_draw_info.h"
#include "ui/ui_draw_scrollbar.h"
#include "ui/ui_draw_text_rect.h"
#include "ui/ui_params.h"

void draw_multiline_text(uint32_t* fb, int fb_w, int fb_h,
                         int x, int y, const char* text,
                         uint32_t color, int base_scale, int line_height, UI_Button* b);
void draw_editor(UI_Button* b, uint32_t* fb, int fb_width, int fb_height, float dt);
void ui_draw(UI_Context* ctx, uint32_t* fb, int fb_width, int fb_height, float dt);
void draw_selection_highlighting(UI_Button* b, uint32_t* fb, int fb_width, int fb_height, int text_x, int draw_y);
void draw_line_numbers(UI_Button* b, uint32_t* fb, int fb_width, int fb_height,
                       int text_x, int draw_y);