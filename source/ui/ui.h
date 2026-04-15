#pragma once
#include "core/events.h"
#include "core/settings.h"
#include "core/time.h"
#include "core/window.h"
#include "ui/ui_context.h"
#include "input/input.h"
#include "scene/sky.h"
#include "ui/ui_callbacks.h"
#include "ui/ui_editor.h"
#include "ui/ui_elements.h"

void ui_init(UI_Context* ctx);
void ui_cleanup(UI_Context* ctx);   // important for freeing memory

void ui_add_button(UI_Context* ctx, int x, int y, int w, int h,
                   const char* text,
                   void (*on_click)(void),
                   void (*on_held)(void),
                   void (*on_release)(void));
// Generic tuner button (left = decrease, right = increase)
void ui_add_tuner(UI_Context* ctx, float x, float y, float w, float h,
                  const char* text,
                  float* target,
                  float min_val,
                  float max_val);
void ui_update(UI_Context* ctx, int mouse_x, int mouse_y, int mouse_pressed, int mouse_wheel, float dt);
void ui_set_mode(UI_Context* ctx, UI_Mode mode);