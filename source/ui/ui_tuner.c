#include "pch.h"
#include "ui/ui_tuner.h"


// for slider-like buttons
void ui_add_tuner(UI_Context* ctx, float x, float y, float w, float h,
                  const char* text,
                  float* target,
                  float min_val,
                  float max_val)
{
    if (ctx->button_count >= get_param_float(PARAM_UI_MAX_BUTTONS)) return;

    UI_Button* b = &ctx->buttons[ctx->button_count];
    
    b->x = x; b->y = y; b->w = w; b->h = h;
    b->content = text; // read-only
    b->editable_content = NULL;
    b->is_editable = false;
    b->on_click = NULL;
    b->on_held = NULL;
    b->on_release = NULL;
    b->is_scrollable = false; // tuner buttons don't have scroll 
    b->is_color = false; // not uint color values but floats
    b->target_value = target;
    b->min_value = min_val;
    b->max_value = max_val;
    // auto-compute (recommended)
    b->step_size = 0.0f;   // mark as "auto"
    
    ctx->button_count++;
}

// for slider-like buttons but based on macros/defines with uint color values instead of floats
void ui_add_macro_tuner(UI_Context* ctx, float x, float y, float w, float h,
                        const char* text,
                        MacroParamID param_id,
                        float min_val,
                        float max_val,
                        bool is_color) // <-- New parameter
{
    if (ctx->button_count >= get_param_float(PARAM_UI_MAX_BUTTONS) || param_id >= PARAM_COUNT) return;

    UI_Button* b = &ctx->buttons[ctx->button_count];
    
    b->x = x; b->y = y; b->w = w; b->h = h;
    b->content = text; 
    b->editable_content = NULL;
    b->is_editable = false;
    b->on_click = NULL;
    b->on_held = NULL;
    b->on_release = NULL;
    b->is_scrollable = false; 
    
    // Bind directly to the persistent storage slot float view
    b->target_value = &s_macro_param_registry[param_id].f;
    
    b->min_value = min_val;
    b->max_value = max_val;
    b->step_size = 0.0f;   
    b->is_color  = is_color; // <-- Store the type flag
    
    ctx->button_count++;
}