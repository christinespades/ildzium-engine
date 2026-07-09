#include "pch.h"
#include "ui/ui.h"

UI_Context* g_ui_ctx = {0};
static float hold_accumulator = 0.0f;
extern void setup_camera_controls(UI_Context* ctx);
extern void setup_fx_controls(UI_Context* ctx);
extern void setup_input_controls(UI_Context* ctx);
extern void setup_lights_controls(UI_Context* ctx);
extern void setup_main_controls(UI_Context* ctx);
extern void setup_meshes_controls(UI_Context* ctx);
extern void setup_project_controls(UI_Context* ctx);
extern void setup_skybox_controls(UI_Context* ctx);
extern void setup_sounds_controls(UI_Context* ctx);
extern void setup_terrain_controls(UI_Context* ctx);
extern void setup_ui_controls(UI_Context* ctx);

#ifndef __EMSCRIPTEN__
    extern GLFWwindow* g_window; 
#endif

float mouse_wheel_sensitivity = 40.0f;

static void add_top_button(UI_Context *ctx,
                           int *x,
                           int top_y,
                           int btn_w,
                           int spacing,
                           const char *label,
                           UI_ButtonCallback on_click,
                           const char* tooltip)
{
    ui_add_button(ctx, *x, top_y, btn_w, 50,
                  label,
                  on_click,
                  on_button_held,
                  NULL,
                  NULL,
                  tooltip);

    *x += btn_w + spacing;
}

#define BUTTON_LIST \
    X("CAMERA",  camera)  \
    X("FX",      fx)      \
    X("INPUT",   input)   \
    X("LIGHTS",  lights)  \
    X("MAIN",    main)    \
    X("MESHES",  meshes)  \
    X("PROJECT", project) \
    X("SKYBOX",  skybox)  \
    X("SOUNDS",  sounds)  \
    X("TERRAIN", terrain) \
    X("UI",      ui)

static void add_top_navigation_buttons(UI_Context* ctx) {
    const int top_y = 5;
    const int btn_w = 200;
    const int spacing = 8;
    int x = get_param_float(PARAM_UI_ELEMENT_START_X);

    #define X(label, cb) \
        add_top_button(ctx, &x, top_y, btn_w, spacing, label, on_##cb##_clicked, tooltip_menu_##cb);

    BUTTON_LIST

    #undef X
}

#define UI_MODES_MAP \
    X(camera,  UI_MODE_CAMERA)  \
    X(fx,      UI_MODE_FX)      \
    X(input,   UI_MODE_INPUT)   \
    X(lights,  UI_MODE_LIGHTS)  \
    X(main,    UI_MODE_MAIN)    \
    X(meshes,  UI_MODE_MESHES)  \
    X(project, UI_MODE_PROJECT) \
    X(skybox,  UI_MODE_SKYBOX)  \
    X(sounds,  UI_MODE_SOUNDS)  \
    X(terrain, UI_MODE_TERRAIN) \
    X(ui,      UI_MODE_UI)

void ui_set_mode(UI_Context* ctx, UI_Mode mode) {
    ctx->current_mode = mode;

    // Clear ALL buttons + reset held state (prevents stale held data on rebuild)
    ctx->button_count = 0;
    memset(ctx->button_held_last_frame, 0, get_param_float(PARAM_UI_MAX_BUTTONS) * sizeof(uint8_t));

    add_top_navigation_buttons(ctx);

    switch (mode) {
        #define X(name, mode_enum)          \
            case mode_enum:                 \
                setup_##name##_controls(ctx); \
                break;

        UI_MODES_MAP
        #undef X

        default:
            break;
    }
}

void ui_init(UI_Context* ctx) {
    ctx->buttons = malloc(sizeof(UI_Button) * get_param_float(PARAM_UI_MAX_BUTTONS));
    ctx->button_held_last_frame = calloc(get_param_float(PARAM_UI_MAX_BUTTONS), sizeof(uint8_t)); // zero-initialized
    ctx->button_count = 0;
#ifdef __EMSCRIPTEN__
    ctx->cursor_captured = 1;
#else
    ctx->cursor_captured = 0;        // start with cursor disabled (camera mode)
#endif

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

void ui_update(UI_Context* ctx, int mouse_x, int mouse_y, int mouse_pressed, int mousewheel, float dt)
{
    if (!ctx->cursor_captured) return;

    static double last_click_time = 0.0;
    static int last_click_button_index = -1;

    for (int i = 0; i < ctx->button_count; i++) {
        UI_Button* b = &ctx->buttons[i];
        int is_hover = (mouse_x >= b->x && mouse_x < b->x + b->w &&
                        mouse_y >= b->y && mouse_y < b->y + b->h);

        // Scroll handling (works for both editors and scrollable panels)
        if (b->is_scrollable && is_hover && mousewheel != 0) {
            b->scroll_offset -= mousewheel * 25.0f;   // adjust sensitivity
        }

        int is_pressed = mouse_pressed && is_hover;

        // ==================== EDITOR MOUSE HANDLING ====================
        if (b->is_editable && is_hover) {

            int pos = get_char_index_from_mouse(b, mouse_x, mouse_y);
            bool is_first_press = is_pressed && !ctx->button_held_last_frame[i];

            if (is_pressed) {   // Mouse button is down (either just clicked or being held)
                double now = ildz_get_time();

                // --- 1. INITIAL CLICK FRAME ---
                if (is_first_press) {
                    b->is_dragging = true;
                    b->drag_start_pos = pos;

                    // === SHIFT + CLICK ===
                    if (platform_get_key_down(KEY_LEFT_SHIFT) || platform_get_key_down(KEY_RIGHT_SHIFT)) {
                        if (b->selection_start == -1) {
                            b->selection_start = b->cursor_pos;
                        }
                        b->selection_end = pos;
                        b->cursor_pos = pos;
                    }
                    // === NORMAL CLICK (Single / Double / Triple) ===
                    else {
                        if (now - b->last_click_time < 0.35 && pos == b->last_click_pos) {
                            b->click_count++;
                        } else {
                            b->click_count = 1;
                        }

                        b->last_click_time = now;
                        b->last_click_pos = pos;

                        if (b->click_count == 1) {
                            // Single click: move caret, reset selection to a starting point
                            b->cursor_pos = pos;
                            b->selection_start = -1;
                            b->selection_end = -1;
                        } 
                        else if (b->click_count == 2) {
                            select_word_at_position(b, pos);
                            b->cursor_pos = b->selection_end;
                        } 
                        else if (b->click_count >= 3) {
                            select_entire_line(b, pos);
                            b->click_count = 0;
                        }
                    }
                }
                // --- 2. DRAGGING / HOLDING FRAMES ---
                else if (b->is_dragging) {
                    // Shift-drag or regular drag on a single-click sequence updates selection
                    if (platform_get_key_down(KEY_LEFT_SHIFT) || platform_get_key_down(KEY_RIGHT_SHIFT)) {
                        b->selection_end = pos;
                        b->cursor_pos = pos;
                    } 
                    else if (b->click_count == 1) {
                        // Regular click-and-drag selection logic
                        if (pos != b->drag_start_pos) {
                            b->selection_start = b->drag_start_pos;
                            b->selection_end = pos;
                        } else {
                            // If the user drags back to the start, clear selection visualizer
                            b->selection_start = -1;
                            b->selection_end = -1;
                        }
                        b->cursor_pos = pos;
                    }
                }

                b->content_height = 0.0f;
            }
            else {
                // Mouse button released
                if (b->is_dragging) {
                    ui_button_mouse_up(b); // sets b->is_dragging = false
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