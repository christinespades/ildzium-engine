#pragma once
#include "ui/ui_borders.h"
#include "ui/ui_draw_text.h"
#include "ui/ui_calculations.h"

void calculate_tooltip_metrics(UI_Context* ctx, UI_Button* temp_b, int line_h, int scale);
void draw_ui_tooltip(UI_Context* ctx, uint32_t* fb, int fb_width, int fb_height);