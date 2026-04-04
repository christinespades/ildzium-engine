#include "ui.h"
#include "events.h"
#include "fonts.h"
#include "settings.h"
#include "sky.h"
#include "sky_ui.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_BUTTONS 64

static float hold_accumulator = 0.0f;   // for smooth acceleration while holding

// Generic event whenever any button is held down
void on_button_held(void) {
    //printf("Button HELD!\n");
}

void ui_init(UI_Context* ctx) {
    ctx->buttons = malloc(sizeof(UI_Button) * MAX_BUTTONS);
    ctx->button_held_last_frame = calloc(MAX_BUTTONS, sizeof(uint8_t)); // zero-initialized
    ctx->button_count = 0;
    ctx->cursor_captured = 0;        // start with cursor disabled (camera mode)

    ui_add_button(ctx, 5, 5, 580, 50, "SKYBOX SETTINGS",
                  on_skybox_settings_button_clicked,
                  on_button_held,
                  on_skybox_settings_button_released);
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

static void draw_pixel(uint32_t* fb, int fb_w, int fb_h, int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= fb_w || y >= fb_h) return;
    fb[y * fb_w + x] = color;
}

static void draw_rect(uint32_t* fb, int fb_w, int fb_h, int x, int y, int w, int h, uint32_t color) {
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            draw_pixel(fb, fb_w, fb_h, x+i, y+j, color);
}

static void draw_char(uint32_t* fb, int fb_w, int fb_h, char c, int x, int y, uint32_t color, int scale) {
    if ((unsigned char)c >= 128) return;
    if (scale < 1) scale = 1;

    for (int row = 0; row < 8; row++) {
        uint8_t bits = font8x8_basic[(unsigned char)c][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << (7 - col))) {
                // Draw scaled pixel block
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        draw_pixel(fb, fb_w, fb_h, x + col*scale + sx, y + row*scale + sy, color);
                    }
                }
            }
        }
    }
}

void draw_text(uint32_t* fb, int fb_w, int fb_h, const char* text, int x, int y, uint32_t color, int scale) {
    int cursor_x = x;
    while (*text) {
        draw_char(fb, fb_w, fb_h, *text++, cursor_x, y, color, scale);
        cursor_x += 8 * scale;
    }
}

void ui_draw(UI_Context* ctx, uint32_t* fb, int fb_width, int fb_height)
{
    if (!ctx->cursor_captured) return;
    memset(fb, 0, (size_t)fb_width * fb_height * sizeof(uint32_t));

    for (int i = 0; i < ctx->button_count; i++) {
        UI_Button* b = &ctx->buttons[i];

        uint32_t panel_color = (b->target_value && ctx->button_held_last_frame[i]) 
                               ? 0xFF555555 : 0xB4000000;

        draw_rect(fb, fb_width, fb_height, b->x, b->y, b->w, b->h, panel_color);

        // borders
        uint32_t border = 0x80FFFFFF;
        draw_rect(fb, fb_width, fb_height, b->x, b->y, b->w, 2, border);
        draw_rect(fb, fb_width, fb_height, b->x, b->y + b->h - 2, b->w, 2, border);
        draw_rect(fb, fb_width, fb_height, b->x, b->y, 2, b->h, border);
        draw_rect(fb, fb_width, fb_height, b->x + b->w - 2, b->y, 2, b->h, border);

        // Optional vertical split line for left/right visual hint
        if (b->target_value) {
            int mid = b->x + b->w / 2;
            draw_rect(fb, fb_width, fb_height, mid, b->y + 4, 2, b->h - 8, 0x00000000);
        }

        // Main label
        if (b->text && b->text[0]) {
            int text_len = (int)strlen(b->text);
            int char_w = 8, char_h = 8;
            int available_w = b->w - 40;           // give space for value
            int scale = 3;

            int scaled_w = text_len * char_w * scale;
            int text_x = b->x + (b->w - scaled_w) / 2;
            int text_y = b->y + 12;
            draw_text(fb, fb_width, fb_height, b->text, text_x, text_y, 0xFFFFFFFF, scale);
        }

        // Current value
        if (b->target_value) {
            char val[32];
            snprintf(val, sizeof(val), "%.3f", *b->target_value);
            draw_text(fb, fb_width, fb_height, val, 
                      b->x + 20, b->y + b->h - 16, 0xFFFFAA00, 2);
        }
    }

    // Draw FPS
    if (g_renderer_flags & RENDERER_SHOW_FPS)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "FPS: %.0f", g_fps);

        draw_text(fb, fb_width, fb_height,
                  buf, 10, 10, 0xFFFFFFFF, 2);
    }
}