#pragma once
#include "ui/ui.h"


#define MAX_BUTTONS 64

void ui_add_button(UI_Context* ctx, int x, int y, int w, int h, const char* text,
                   UI_ButtonCallback on_click,
                   UI_ButtonCallback on_held,
                   UI_ButtonCallback on_release);

void ui_add_scrollable_text(UI_Context* ctx, int x, int y, int w, int h, const char* text);
void ui_add_scrollable_text_editor(UI_Context* ctx, int x, int y, int w, int h, const char* initial_text, const char* filepath);
void ui_add_tuner(UI_Context* ctx, float x, float y, float w, float h,
                  const char* text,
                  float* target,
                  float min_val,
                  float max_val);