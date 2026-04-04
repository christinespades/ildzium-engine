#include "core/fonts.h"
#include "core/settings.h"
#include "ui/ui_draw.h"
#include "ui/ui.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
    int cursor_y = y;
    while (*text) {
        if (*text == '\n') {
            cursor_y += 8 * scale;   // move down one line
            cursor_x = x;            // reset horizontal
            text++;
            continue;
        }
        draw_char(fb, fb_w, fb_h, *text++, cursor_x, cursor_y, color, scale);
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
            int char_w = 8, char_h = 8;
            int padding = 10;

            // First, count lines and longest line
            int max_line_len = 0;
            int num_lines = 1;
            const char* p = b->text;
            int line_len = 0;
            while (*p) {
                if (*p == '\n') {
                    if (line_len > max_line_len) max_line_len = line_len;
                    line_len = 0;
                    num_lines++;
                } else {
                    line_len++;
                }
                p++;
            }
            if (line_len > max_line_len) max_line_len = line_len;

            // Compute maximum scale that fits horizontally and vertically
            int scale_x = (b->w - 2*padding) / (max_line_len * char_w);
            int scale_y = (b->h - 2*padding) / (num_lines * char_h);
            int scale = scale_x < scale_y ? scale_x : scale_y;
            if (scale < 1) scale = 1;
            if (scale > 2) scale = 2;

            // Draw each line centered
            int cursor_y = b->y + padding;
            char* text_copy = strdup(b->text); // strtok modifies string
            char* line = strtok(text_copy, "\n");
            while (line) {
                int line_px_w = (int)strlen(line) * char_w * scale;
                int text_x = b->x + (b->w - line_px_w)/2;
                draw_text(fb, fb_width, fb_height, line, text_x, cursor_y, 0xFFFFFFFF, scale);
                cursor_y += char_h * scale;
                line = strtok(NULL, "\n");
            }
            free(text_copy);
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