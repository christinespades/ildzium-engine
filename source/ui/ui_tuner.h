#pragma once
#include "ui/ui_context.h"
#include "core/params/params.h"

void ui_add_tuner_float(UI_Context* ctx, float x, float y, float w, float h,
                  const char* text,
                  MacroParamID param_id,
                  float min_val,
                  float max_val,
                  const char* tooltip);
void ui_add_tuner_bool(UI_Context* ctx, float x, float y, float w, float h,
                        const char* text,
                        MacroParamID param_id,
                        const char* tooltip);
void ui_add_tuner_enum(
    UI_Context* ctx,
    float x,
    float y,
    float w,
    float h,
    const char* text,
    MacroParamID param_id,
    const EnumDefinition* enum_definition,
    const char* tooltip);
// The vec3 overload function
void ui_add_tuner_v3(UI_Context* ctx, float x, float y, float w, float h, 
                           const char* name, int param_id, vec3 min_val, vec3 max_val, 
                           const char* tooltip, vec3* out_vec3);

// The vec4 overload function
void ui_add_tuner_v4(UI_Context* ctx, float x, float y, float w, float h, 
                           const char* name, int param_id, vec4 min_val, vec4 max_val, 
                           const char* tooltip, vec4* out_vec4);