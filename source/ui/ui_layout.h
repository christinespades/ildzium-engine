#pragma once
#include "core/params/params.h"
#include "ui/ui_callbacks.h"

typedef struct UI_Button UI_Button;
typedef struct UI_Context UI_Context;
typedef struct {
    float btn_w;
    float btn_h;
    float start_y;
    float spacing;
    float current_x;
    float max_bottom_y;
    int idx;
} UI_Layout;

UI_Layout ui_layout_begin(UI_Context* ctx);
UI_Layout ui_layout_begin_projects(UI_Context* ctx, int dynamic_item_count);
float ui_layout_next_y(UI_Layout* l);