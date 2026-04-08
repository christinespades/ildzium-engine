#pragma once
#include "core/events.h"
#include "core/settings.h"
#include "scene/sky.h"
#include "ui/ui_callbacks.h"
#include "ui/ui_editor.h"

typedef enum {
    UI_MODE_CAMERA = 0,
    UI_MODE_FX,
    UI_MODE_INPUT,
    UI_MODE_LIGHTS,
    UI_MODE_MAIN,
    UI_MODE_MESHES,
    UI_MODE_SKYBOX,
    UI_MODE_SOUNDS,
    UI_MODE_TERRAIN,
    UI_MODE_COUNT
} UI_Mode;

#include "ui/ui_button.h"

typedef struct UI_Context {
    UI_Button* buttons;
    int button_count;
    int cursor_captured;
    uint8_t* button_held_last_frame;
    UI_Mode current_mode;
} UI_Context;

UI_Context* g_ui_ctx;   // global so callbacks can reach it

#include "ui/ui_elements.h"
#include "ui/modes/ui_modes.h"

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