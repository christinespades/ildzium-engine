#include "pch.h"
#include "ui/ui_tuner.h"
#include "ui/ui_button.h"
#include "core/params/param_type.h"

// for slider-like buttons
void ui_add_tuner_float(UI_Context* ctx, float x, float y, float w, float h,
                  const char* text,
                  MacroParamID param_id,
                  float min_val,
                  float max_val,
                  const char* tooltip)
{
    if (ctx->button_count >= get_param_float(PARAM_UI_MAX_BUTTONS)) return;

    UI_Button* b = &ctx->buttons[ctx->button_count];
    
    b->x = x; b->y = y; b->w = w; b->h = h;
    b->content = text; // read-only context prefix or label
    
    // --- Setup Editable Capabilities ---
    b->is_editable = true;
    b->editable_content = NULL;
    b->selection_start = -1;
    b->selection_end = -1;
    b->filepath = NULL; // Not linked to a file
    b->on_click = NULL;
    b->on_held = NULL;
    b->on_release = NULL;
    b->is_scrollable = false; 
    b->tooltip = tooltip;
    b->target_value = &s_macro_param_registry[param_id].f;
    b->type = PARAM_TYPE_FLOAT;
    b->min_value = min_val;
    b->max_value = max_val;
    b->step_size = 0.0f;   
    b->is_typing = false; 
    b->undo_stack = NULL; 

    //init_editor_undo(b, 2); // Minimal undo frames for numeric fields
    
    ctx->button_count++;
}

// for slider-like buttons but based on macros/defines with uint color values instead of floats
void ui_add_tuner_bool(UI_Context* ctx, float x, float y, float w, float h,
                        const char* text,
                        MacroParamID param_id,
                        const char* tooltip)
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
    b->tooltip = tooltip;
    b->target_value = &s_macro_param_registry[param_id].b;
    b->type = PARAM_TYPE_BOOL;
    b->min_value = false;
    b->max_value = true;
    b->step_size = 0.0f;
    b->is_typing = false; 
    b->undo_stack = NULL; 
    ctx->button_count++;
}

void ui_add_tuner_v3(UI_Context* ctx, float x, float y, float w, float h,
                           const char* text,
                           MacroParamID param_id,
                           vec3 min_val,
                           vec3 max_val,
                           const char* tooltip,
                           vec3* target_vec3)
{
    // A vec3 takes up 3 button slots. Make sure we have space!
    if (ctx->button_count + 3 > get_param_float(PARAM_UI_MAX_BUTTONS) || param_id >= PARAM_COUNT) return;

    // Split the available width across 3 sub-tuners
    float component_w = w / 3.0f;

    // We store min/max sub-components into temporary arrays to iterate over them
    float mins[3] = { min_val.x, min_val.y, min_val.z };
    float maxs[3] = { max_val.x, max_val.y, max_val.z };
    float* targets[3] = { &target_vec3->x, &target_vec3->y, &target_vec3->z };

    for (int i = 0; i < 3; i++)
    {
        UI_Button* b = &ctx->buttons[ctx->button_count];
        
        // Offset each component horizontally next to each other
        b->x = x + (i * component_w); 
        b->y = y; 
        b->w = component_w; 
        b->h = h;
        
        b->content = text; 
        b->editable_content = NULL;
        b->is_editable = false;
        b->on_click = NULL;
        b->on_held = NULL;
        b->on_release = NULL;
        b->is_scrollable = false; 
        b->tooltip = tooltip;
        
        b->target_value = targets[i];
        b->type = PARAM_TYPE_FLOAT;
        b->min_value = mins[i];
        b->max_value = maxs[i];
        b->step_size = 0.0f;   
        ctx->button_count++;
    }
}

void ui_add_tuner_v4(UI_Context* ctx, float x, float y, float w, float h,
                           const char* text,
                           MacroParamID param_id,
                           vec4 min_val,
                           vec4 max_val,
                           const char* tooltip,
                           vec4* target_vec4)
{
    // A vec4 takes up 4 button slots. Make sure we have space!
    if (ctx->button_count + 4 > get_param_float(PARAM_UI_MAX_BUTTONS) || param_id >= PARAM_COUNT) return;

    // Split the available width across 4 sub-tuners (R, G, B, A)
    float component_w = w / 4.0f;

    float mins[4] = { min_val.x, min_val.y, min_val.z, min_val.w };
    float maxs[4] = { max_val.x, max_val.y, max_val.z, max_val.w };
    float* targets[4] = { &target_vec4->x, &target_vec4->y, &target_vec4->z, &target_vec4->w };

    for (int i = 0; i < 4; i++)
    {
        UI_Button* b = &ctx->buttons[ctx->button_count];
        
        // Offset each component horizontally next to each other
        b->x = x + (i * component_w); 
        b->y = y; 
        b->w = component_w; 
        b->h = h;
        
        b->content = text; 
        b->editable_content = NULL;
        b->is_editable = false;
        b->on_click = NULL;
        b->on_held = NULL;
        b->on_release = NULL;
        b->is_scrollable = false; 
        b->tooltip = tooltip;
        
        b->target_value = targets[i];
        b->type = PARAM_TYPE_FLOAT;
        b->min_value = mins[i];
        b->max_value = maxs[i];
        b->step_size = 0.0f;   
        ctx->button_count++;
    }
}

#include "core/enums.h"

void ui_add_tuner_enum(
    UI_Context* ctx,
    float x,
    float y,
    float w,
    float h,
    const char* text,
    MacroParamID param_id,
    const EnumDefinition* enum_definition,
    const char* tooltip)
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
    b->tooltip = tooltip;
    b->target_value = &s_macro_param_registry[param_id].e;
    b->type = PARAM_TYPE_ENUM;
    b->min_value = 0.0f;
    b->max_value = enum_definition->count - 1;
    b->enum_definition = enum_definition;
    b->step_size = 1.0f;
    b->is_typing = false; 
    b->undo_stack = NULL; 
    ctx->button_count++;
}