#include "ui/ui.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "core/events.h"
#include "core/settings.h"
#include "scene/sky.h"
#include "ui_callbacks.h"
#include "ui/ui_editor.h"
#include "ui/ui_elements.h"
#include "ui/modes/ui_skybox.h"

static float hold_accumulator = 0.0f;
extern void setup_main_menu_controls(UI_Context* ctx);
extern void setup_terrain_controls(UI_Context* ctx);

float mouse_wheel_sensitivity = 40.0f;

static void add_top_navigation_buttons(UI_Context* ctx) {
    const int top_y = 5;
    const int btn_w = 200;
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

    add_top_navigation_buttons(ctx);

    switch (mode) {
        case UI_MODE_MAIN_MENU:
            setup_main_menu_controls(ctx);
            break;

        case UI_MODE_SKYBOX:
            setup_skybox_controls(ctx);
            break;

        case UI_MODE_TERRAIN:
            setup_terrain_controls(ctx);
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

void ui_update(UI_Context* ctx, int mouse_x, int mouse_y, int mouse_pressed, int mouse_wheel, float dt)
{
    if (!ctx->cursor_captured) return;

    static double last_click_time = 0.0;
    static int last_click_button_index = -1;
    double current_time = glfwGetTime();

    for (int i = 0; i < ctx->button_count; i++) {
        UI_Button* b = &ctx->buttons[i];
        int is_hover = (mouse_x >= b->x && mouse_x < b->x + b->w &&
                        mouse_y >= b->y && mouse_y < b->y + b->h);

        // Scroll handling (works for both editors and scrollable panels)
        if (b->is_scrollable && is_hover && mouse_wheel != 0) {
            b->scroll_offset -= mouse_wheel * 25.0f;   // adjust sensitivity
        }

        int is_pressed = mouse_pressed && is_hover;

        // ==================== EDITOR MOUSE HANDLING ====================
        if (b->is_editable && is_hover) {

            // Left click - set cursor / start selection
            if (is_pressed) {
                int new_cursor = get_char_index_from_mouse(b, mouse_x, mouse_y);

                // Double-click detection
                if (current_time - last_click_time < 0.35 && last_click_button_index == i) {
                    // Double click → select word
                    select_word_at_position(b, new_cursor);
                } else {
                    // Single click
                    b->cursor_pos = new_cursor;
                    b->selection_start = b->selection_end = new_cursor;   // start new selection
                }

                last_click_time = current_time;
                last_click_button_index = i;
            }
            // Mouse drag → update selection end
            else if (ctx->button_held_last_frame[i] && mouse_pressed) {
                int new_pos = get_char_index_from_mouse(b, mouse_x, mouse_y);
                b->selection_end = new_pos;
                b->cursor_pos = new_pos;   // cursor follows mouse during drag
            }
        }

        // === TUNING LOGIC (while held) ===
        if (is_pressed && b->target_value != NULL) {
            if (!ctx->button_held_last_frame[i]) b->hold_time = 0.0f;  // reset per button on new press

            b->hold_time += dt;  // accumulate seconds held

            // Quadratic acceleration: starts very small, gets faster the longer you hold
            float speed = 0.1f + b->hold_time * b->hold_time * 9.0f;  // adjust 9.0f for faster/farther

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
        if (is_pressed && !ctx->button_held_last_frame[i]) {
            if (b->on_click) b->on_click();
        }
        if (!is_pressed && ctx->button_held_last_frame[i]) {
            if (b->on_release) b->on_release();
        }

        ctx->button_held_last_frame[i] = is_pressed;
    }
}