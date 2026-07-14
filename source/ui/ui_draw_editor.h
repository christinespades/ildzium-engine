#pragma once
#include "rendering/fps.h"
#include "ui/ui.h"
#include "ui/ui_draw_text.h"

void draw_editor(UI_Button* b);
void draw_selection_highlighting(UI_Button* b, uint32_t* fb, int fb_width, int fb_height, int text_x, int draw_y);
void draw_line_numbers(UI_Button* b, uint32_t* fb, int fb_width, int fb_height,
                       int text_x, int draw_y);