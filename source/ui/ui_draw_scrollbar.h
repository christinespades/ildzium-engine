#pragma once
#include "ui/ui.h"
#include "core/colors.h"
#include "ui/ui_draw_text_rect.h"

void draw_scrollbar(UI_Button* b, uint32_t* fb, int fb_width, int fb_height);
void draw_horizontal_scrollbar(UI_Button* b, uint32_t* fb, int fb_width, int fb_height);