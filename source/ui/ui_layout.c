#include "pch.h"
#include "ui/ui_layout.h"
#include "ui/ui_context.h"

// Initializes the layout tracking variables from parameters
static UI_Layout internal_layout_init(UI_Context* ctx, float spacing_adjustment, float width_adjustment, float content_height) {
    UI_Layout l;
    l.btn_w        = get_param_float(PARAM_UI_TUNER_WIDTH);
    l.btn_h        = get_param_float(PARAM_UI_TUNER_HEIGHT);
    l.current_x    = get_param_float(PARAM_UI_ELEMENT_START_X);
    l.start_y      = get_param_float(PARAM_UI_ELEMENT_START_Y);
    l.spacing      = l.btn_h + spacing_adjustment;
    l.idx          = 0;
    
    float max_box_h = get_param_float(PARAM_UI_EDITOR_HEIGHT);
    l.max_bottom_y  = max_box_h - l.btn_h;

    if (ctx->button_count < get_param_float(PARAM_UI_MAX_BUTTONS)) {
        UI_Button* container = &ctx->buttons[ctx->button_count];
        
        container->x = l.current_x;
        container->y = l.start_y;
        container->w = l.btn_w + width_adjustment;
        container->h = max_box_h - l.start_y;
        
        container->content = NULL; 
        container->editable_content = NULL;
        container->is_editable = false;
        container->on_click = NULL;
        container->on_held = NULL;
        container->on_release = NULL;
        
        container->is_scrollable = true; 
        container->target_value = NULL;
        container->content_height = content_height; 
        container->content_width = container->w;
        container->is_typing = false; // only for tuner text editor boxes
        // Explicitly flag this as a background container element
        container->is_container = true; 

        ctx->button_count++;
    }

    return l;
}

UI_Layout ui_layout_begin(UI_Context* ctx) {
    float stride = get_param_float(PARAM_UI_TUNER_HEIGHT);
    return internal_layout_init(ctx, 0.0f, 0.0f, 32.0f * stride);
}

UI_Layout ui_layout_begin_projects(UI_Context* ctx, int dynamic_item_count) {
    float stride = get_param_float(PARAM_UI_TUNER_HEIGHT) + 10.0f;
    float expected_height = (dynamic_item_count + 2) * stride;
    return internal_layout_init(ctx, 10.0f, 160.0f, expected_height);
}

float ui_layout_next_y(UI_Layout* l) {
    float test_y = l->start_y + (l->idx * l->spacing);
    if (test_y >= l->max_bottom_y) {
        l->current_x += l->btn_w + 40.0f; 
        l->idx = 0;                        
        test_y = l->start_y;
    }
    l->idx++;
    return test_y;
}