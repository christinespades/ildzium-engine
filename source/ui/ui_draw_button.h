#pragma once
#include "ui/ui.h"
#include "ui/ui_borders.h"
#include "ui/ui_calculations.h"
#include "ui/ui_draw_scrollbar.h"
#include "ui/ui_draw_editor.h"
#include "ui/ui_draw_rect_panel.h"

void draw_ui_single_button(UI_Context* ctx, UI_Button* b, uint32_t* fb, int fb_width, int fb_height, int index, float dt);