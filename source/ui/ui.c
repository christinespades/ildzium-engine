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
#include "ui/modes/ui_modes.h"

static float hold_accumulator = 0.0f;
extern void setup_camera_controls(UI_Context* ctx);
extern void setup_fx_controls(UI_Context* ctx);
extern void setup_input_controls(UI_Context* ctx);
extern void setup_lights_controls(UI_Context* ctx);
extern void setup_main_controls(UI_Context* ctx);
extern void setup_meshes_controls(UI_Context* ctx);
extern void setup_skybox_controls(UI_Context* ctx);
extern void setup_sounds_controls(UI_Context* ctx);
extern void setup_terrain_controls(UI_Context* ctx);

extern GLFWwindow* g_window; 

float mouse_wheel_sensitivity = 40.0f;

static void add_top_button(UI_Context *ctx,
                           int *x,
                           int top_y,
                           int btn_w,
                           int spacing,
                           const char *label,
                           UI_ButtonCallback on_click)
{
    ui_add_button(ctx, *x, top_y, btn_w, 50,
                  label,
                  on_click,
                  on_button_held,
                  NULL);

    *x += btn_w + spacing;
}

static void add_top_navigation_buttons(UI_Context* ctx) {
    const int top_y = 5;
    const int btn_w = 200;
    const int spacing = 8;
    int x = 10;

    add_top_button(ctx, &x, top_y, btn_w, spacing, "CAMERA",  on_camera_clicked);
    add_top_button(ctx, &x, top_y, btn_w, spacing, "FX",      on_fx_clicked);
    add_top_button(ctx, &x, top_y, btn_w, spacing, "INPUT",   on_input_clicked);
    add_top_button(ctx, &x, top_y, btn_w, spacing, "LIGHTS",  on_lights_clicked);
    add_top_button(ctx, &x, top_y, btn_w, spacing, "MAIN",    on_main_clicked);
    add_top_button(ctx, &x, top_y, btn_w, spacing, "MESHES",  on_meshes_clicked);
    add_top_button(ctx, &x, top_y, btn_w, spacing, "SKYBOX",  on_skybox_clicked);
    add_top_button(ctx, &x, top_y, btn_w, spacing, "SOUNDS",  on_sounds_clicked);
    add_top_button(ctx, &x, top_y, btn_w, spacing, "TERRAIN", on_terrain_clicked);
}

void ui_set_mode(UI_Context* ctx, UI_Mode mode) {
    ctx->current_mode = mode;

    // Clear ALL buttons + reset held state (prevents stale held data on rebuild)
    ctx->button_count = 0;
    memset(ctx->button_held_last_frame, 0, MAX_BUTTONS * sizeof(uint8_t));

    add_top_navigation_buttons(ctx);

    switch (mode) {
        case UI_MODE_CAMERA:
            setup_camera_controls(ctx);
            break;

        case UI_MODE_FX:
            setup_fx_controls(ctx);
            break;
            
        case UI_MODE_INPUT:
            setup_input_controls(ctx);
            break;

        case UI_MODE_LIGHTS:
            setup_lights_controls(ctx);
            break;

        case UI_MODE_MAIN:
            setup_main_controls(ctx);
            break;

        case UI_MODE_MESHES:
            setup_meshes_controls(ctx);
            break;

        case UI_MODE_SKYBOX:
            setup_skybox_controls(ctx);
            break;

        case UI_MODE_SOUNDS:
            setup_sounds_controls(ctx);
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
    ctx->current_mode = UI_MODE_MAIN;

    ui_set_mode(ctx, UI_MODE_MAIN);   // builds top bar + main menu
}

void ui_cleanup(UI_Context* ctx) {
    if (ctx->buttons) free(ctx->buttons);
    if (ctx->button_held_last_frame) free(ctx->button_held_last_frame);
    ctx->buttons = NULL;
    ctx->button_held_last_frame = NULL;
    ctx->button_count = 0;
}

// New helper: select entire current line
void select_entire_line(UI_Button* b, int click_pos)
{
    const char* text = b->editable_content;
    if (!text) return;

    int len = (int)strlen(text);

    // Find start of line
    int start = click_pos;
    while (start > 0 && text[start-1] != '\n') start--;

    // Find end of line
    int end = click_pos;
    while (end < len && text[end] != '\n') end++;

    b->selection_start = start;
    b->selection_end = end;
    b->cursor_pos = end;
}

// Mouse motion (dragging)
void ui_button_mouse_drag(UI_Button* b, int mouse_x, int mouse_y)
{
    if (!b->is_dragging || !b->is_editable) return;

    int current_pos = get_char_index_from_mouse(b, mouse_x, mouse_y);

    if (b->click_count == 1)   // normal drag selection
    {
        b->selection_start = b->drag_start_pos;
        b->selection_end = current_pos;
        b->cursor_pos = current_pos;
    }
    else if (b->click_count == 2)   // word-by-word drag after double-click
    {
        // Optional: expand word selection while dragging (many editors do this)
        // For simplicity you can keep normal selection here
        b->selection_start = b->drag_start_pos;
        b->selection_end = current_pos;
        b->cursor_pos = current_pos;
    }
}

void ui_button_mouse_up(UI_Button* b)
{
    b->is_dragging = false;
}

/*
something weird. the selection highlight appears transparent not black, but i like it, but it masks the text so it cant be read, i thought the text would be visible on top? and the selection rect flickers, sometimes just a single click on a world selects the whole line, other times it just selects the word. however the shift + select works perfectly, flawlessly, selecting multiple lines etc, no flickering there


fps bar 

wrap text properly


*/
void ui_update(UI_Context* ctx, int mouse_x, int mouse_y, int mouse_pressed, int mouse_wheel, float dt)
{
    if (!ctx->cursor_captured) return;

    static double last_click_time = 0.0;
    static int last_click_button_index = -1;

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

            int pos = get_char_index_from_mouse(b, mouse_x, mouse_y);

            if (is_pressed) {   // mouse button is down this frame
                double now = glfwGetTime();
                b->is_dragging = true;
                b->drag_start_pos = pos;

                // === SHIFT + CLICK / DRAG ===
                if (glfwGetKey(g_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                    glfwGetKey(g_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
                {
                    if (b->selection_start == -1)
                        b->selection_start = b->cursor_pos;

                    b->selection_end = pos;
                    b->cursor_pos = pos;
                }
                // === Normal click / double / triple click ===
                else if (!b->is_dragging || /* first press */ !ctx->button_held_last_frame[i])
                {
                    // Only count clicks on the initial press, not while holding
                    if (now - b->last_click_time < 0.35 && pos == b->last_click_pos)
                    {
                        b->click_count++;
                    }
                    else
                    {
                        b->click_count = 1;
                    }

                    b->last_click_time = now;
                    b->last_click_pos = pos;

                    if (b->click_count == 1)
                    {
                        // Single click: move caret, clear selection
                        b->cursor_pos = pos;
                        b->selection_start = b->selection_end = -1;
                    }
                    else if (b->click_count == 2)
                    {
                        // Double click: select word
                        select_word_at_position(b, pos);
                        b->cursor_pos = b->selection_end;
                    }
                    else if (b->click_count >= 3)
                    {
                        // Triple click: select entire line
                        select_entire_line(b, pos);
                        b->click_count = 0;
                    }
                }
                // While dragging (button held) after single click → update selection
                else if (b->click_count == 1 && b->is_dragging)
                {
                    b->selection_start = b->drag_start_pos;
                    b->selection_end = pos;
                    b->cursor_pos = pos;
                }

                b->content_height = 0.0f;
            }
            else {
                // Mouse button released
                if (b->is_dragging) {
                    ui_button_mouse_up(b);   // your existing function that sets is_dragging = false
                }
            }

            // Important: Do NOT return here for editable buttons!
            // We still want to run the general button logic below if needed
            // Only skip if you have a good reason
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
            ui_button_mouse_up(b);
            if (b->on_release) b->on_release();
        }

        ctx->button_held_last_frame[i] = is_pressed;
    }
}