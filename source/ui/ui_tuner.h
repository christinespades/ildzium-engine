#pragma once
#include "ui/ui_context.h"
#include "ui/ui_params.h"

void ui_add_tuner(UI_Context* ctx, float x, float y, float w, float h,
                  const char* text,
                  float* target,
                  float min_val,
                  float max_val);
void ui_add_macro_tuner(UI_Context* ctx, float x, float y, float w, float h,
                        const char* text,
                        MacroParamID param_id,
                        float min_val,
                        float max_val,
                        bool is_color);
