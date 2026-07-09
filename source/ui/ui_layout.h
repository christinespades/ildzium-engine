#pragma once
#include "ui/ui_params.h"
#include "ui/ui_callbacks.h"

typedef struct {
    float btn_w;
    float btn_h;
    float start_y;
    float spacing;
    float current_x;
    float max_bottom_y;
    int idx;
} UI_Layout;

// Initializes the layout tracking variables from parameters
static inline UI_Layout ui_layout_begin(UI_Context* ctx) {
    UI_Layout l;
    l.btn_w        = get_param_float(PARAM_UI_TUNER_WIDTH);
    l.btn_h        = get_param_float(PARAM_UI_TUNER_HEIGHT);
    l.current_x    = get_param_float(PARAM_UI_ELEMENT_START_X);
    l.start_y      = get_param_float(PARAM_UI_ELEMENT_START_Y);
    l.spacing      = l.btn_h;
    l.idx          = 0;
    
    // Bottom edge constraint bounds
    float max_box_h = get_param_float(PARAM_UI_EDITOR_HEIGHT);
    l.max_bottom_y  = max_box_h - l.btn_h;

    // --- Create Layout Housing Box Container ---
    if (ctx->button_count < get_param_float(PARAM_UI_MAX_BUTTONS)) {
        UI_Button* container = &ctx->buttons[ctx->button_count];
        
        container->x = l.current_x;
        container->y = l.start_y;
        container->w = l.btn_w;
        container->h = max_box_h - l.start_y; // Box spans completely to bottom limit
        
        container->content = NULL; // Keeps normal button text loops from executing on it
        container->editable_content = NULL;
        container->is_editable = false;
        container->on_click = on_layout_box_clicked;
        container->on_held = NULL;
        container->on_release = NULL;
        
        // This flags the render engine to track inner overflow parameters and display sliders
        container->is_scrollable = true; 
        container->target_value = NULL;
        
        // Compute total combined dynamic height of all incoming elements mapped to this region
        // We multiply total macro elements by your spacing stride to define virtual content size
        // Replace '32' below with your maximum total tuner elements if dynamically tracked
        container->content_height = 32 * l.spacing; 
        container->content_width = l.btn_w;

        ctx->button_count++;
    }

    return l;
}

// Processes column wrapping math and returns the next valid rendering Y coordinate
static inline float ui_layout_next_y(UI_Layout* l) {
    float test_y = l->start_y + (l->idx * l->spacing);
    if (test_y >= l->max_bottom_y) {
        l->current_x += l->btn_w + 40.0f; // Shift column
        l->idx = 0;                        // Reset row
        test_y = l->start_y;
    }
    l->idx++;
    return test_y;
}