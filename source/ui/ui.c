#include "ui/ui.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "core/events.h"
#include "core/settings.h"
#include "scene/sky.h"
#include "ui_callbacks.h"
#include "ui/ui_skybox.h"

#define MAX_BUTTONS 64

static float hold_accumulator = 0.0f;
extern void setup_main_menu_controls(UI_Context* ctx);
extern void setup_terrain_controls(UI_Context* ctx);

static void add_top_navigation_buttons(UI_Context* ctx) {
    const int top_y = 5;
    const int btn_w = 130;
    const int spacing = 8;
    int x = 10;

    // Always present in every context
    ui_add_button(ctx, x, top_y, btn_w, 50, "MAIN MENU",
                  on_main_menu_clicked, on_button_held, NULL);
    x += btn_w + spacing;

    ui_add_button(ctx, x, top_y, btn_w, 50, "SKYBOX",
                  on_skybox_clicked, on_button_held, NULL);
    x += btn_w + spacing;

    ui_add_button(ctx, x, top_y, btn_w, 50, "TERRAIN",
                  on_terrain_clicked, on_button_held, NULL);
    x += btn_w + spacing;

    // add more tabs here later (LIGHTS, FOG, etc.)
}

void ui_set_mode(UI_Context* ctx, UI_Mode mode) {
    ctx->current_mode = mode;

    // Clear ALL buttons + reset held state (prevents stale held data on rebuild)
    ctx->button_count = 0;
    memset(ctx->button_held_last_frame, 0, MAX_BUTTONS * sizeof(uint8_t));

    // 1. Re-add the top navigation (always first)
    add_top_navigation_buttons(ctx);

    // 2. Add the context-specific controls below the top bar
    switch (mode) {
        case UI_MODE_MAIN_MENU:
            setup_main_menu_controls(ctx);   // you will write this
            break;

        case UI_MODE_SKYBOX:
            setup_skybox_controls(ctx);      // renamed from your old setup_sky_tuners
            break;

        case UI_MODE_TERRAIN:
            setup_terrain_controls(ctx);     // you will write this
            break;

        default:
            break;
    }
}

void ui_init(UI_Context* ctx) {
    ctx->buttons = malloc(sizeof(UI_Button) * MAX_BUTTONS);
    ctx->button_held_last_frame = calloc(MAX_BUTTONS, sizeof(uint8_t)); // zero-initialized
    ctx->button_count = 0;
    ctx->cursor_captured = 0;        // start with cursor disabled (camera mode)

    g_ui_ctx = ctx;                    // <-- important for callbacks
    ctx->current_mode = UI_MODE_MAIN_MENU;

    ui_set_mode(ctx, UI_MODE_MAIN_MENU);   // builds top bar + main menu
}

void ui_cleanup(UI_Context* ctx) {
    if (ctx->buttons) free(ctx->buttons);
    if (ctx->button_held_last_frame) free(ctx->button_held_last_frame);
    ctx->buttons = NULL;
    ctx->button_held_last_frame = NULL;
    ctx->button_count = 0;
}

void ui_add_button(UI_Context* ctx, int x, int y, int w, int h, const char* text,
                   UI_ButtonCallback on_click,
                   UI_ButtonCallback on_held,
                   UI_ButtonCallback on_release) {
    if (ctx->button_count >= MAX_BUTTONS) return;

    UI_Button* b = &ctx->buttons[ctx->button_count];
    b->x = x; b->y = y; b->w = w; b->h = h;
    b->text = text;
    b->on_click = on_click;
    b->on_held = on_held;
    b->on_release = on_release;

    // Default: no tuning
    b->target_value = NULL;
    b->step_size = 0.0f;
    b->min_value = 0.0f;
    b->max_value = 0.0f;

    ctx->button_count++;
}

void ui_add_tuner(UI_Context* ctx, float x, float y, float w, float h,
                  const char* text,
                  float* target,
                  float min_val,
                  float max_val)
{
    if (ctx->button_count >= MAX_BUTTONS) return;

    UI_Button* b = &ctx->buttons[ctx->button_count];
    
    b->x = x; b->y = y; b->w = w; b->h = h;
    b->text = text;
    b->on_click = NULL;
    b->on_held = NULL;
    b->on_release = NULL;
    b->target_value = target;
    b->min_value = min_val;
    b->max_value = max_val;
    // auto-compute (recommended)
    b->step_size = 0.0f;   // mark as "auto"
    
    ctx->button_count++;
}

void ui_update(UI_Context* ctx, int mouse_x, int mouse_y, int mouse_pressed, float dt)
{
    if (!ctx->cursor_captured) return;

    for (int i = 0; i < ctx->button_count; i++) {
        UI_Button* b = &ctx->buttons[i];
        uint8_t* was_held = &ctx->button_held_last_frame[i];

        int is_hover = (mouse_x >= b->x && mouse_x < b->x + b->w &&
                        mouse_y >= b->y && mouse_y < b->y + b->h);

        int is_pressed = mouse_pressed && is_hover;

        // === TUNING LOGIC (while held) ===
        if (is_pressed && b->target_value != NULL) {
            if (!*was_held) b->hold_time = 0.0f;  // reset per button on new press

            b->hold_time += dt;  // accumulate seconds held

            // Quadratic acceleration: starts very small, gets faster the longer you hold
            float speed = 0.1f + b->hold_time * b->hold_time * 3.0f;  // adjust 3.0f for faster/farther

            float range = b->max_value - b->min_value;
            
            float step;
            if (b->step_size > 0.0f) {
                // User explicitly wants a fixed step (rare)
                step = b->step_size;
            } else {
                // Automatic proportional step - this is what you want
                step = range * 0.1f;
            }

            float delta = step * speed * dt;  // scale by dt

            if (mouse_x < b->x + b->w / 2) {
                *b->target_value -= delta;  // left decreases
            } else {
                *b->target_value += delta;  // right increases
            }

            // Clamp to limits
            if (*b->target_value < b->min_value) *b->target_value = b->min_value;
            if (*b->target_value > b->max_value) *b->target_value = b->max_value;
        } else if (!is_pressed && b->target_value != NULL) {
            b->hold_time = 0.0f;  // reset when released
        }

        // === Original callback behavior (still works) ===
        if (is_pressed) {
            if (b->on_held) b->on_held();
        }

        if (is_pressed && !(*was_held)) {
            if (b->on_click) b->on_click();
        }

        if (!is_pressed && *was_held) {
            if (b->on_release) b->on_release();
        }

        *was_held = is_pressed;
    }
}