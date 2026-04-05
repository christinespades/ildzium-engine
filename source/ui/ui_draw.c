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

void draw_multiline_text(uint32_t* fb, int fb_w, int fb_h,
                         int x, int y, const char* text,
                         uint32_t color, int base_scale, int line_height)
{
    if (!text || !*text) return;

    char line[512];
    const char* p = text;
    int current_y = y;

    while (*p) {
        int i = 0;
        while (*p && *p != '\n' && i < 510) {
            line[i++] = *p++;
        }
        line[i] = '\0';

        if (i > 0) {
            // Check for header control character at start of line
            int draw_scale = base_scale;
            int offset_x = 0;
            const char* lines_to_draw = line;

            if (line[0] >= 0x02 && line[0] <= 0x07) {  // our header marker
                int level = line[0] - 0x01;
                draw_scale = (level == 1) ? 2 : (level <= 3 ? 1 : base_scale);  // H1=2x, H2/H3=1x, smaller=normal
                lines_to_draw = line + 1;  // skip the control char
                offset_x = 4;          // small indent for headers
            }

            draw_text(fb, fb_w, fb_h, lines_to_draw, x + offset_x, current_y, color, draw_scale);
        }

        current_y += line_height;
        if (*p == '\n') p++;
    }
}

// ====================== DRAW HELPERS ======================
void draw_editor(UI_Button* b, uint32_t* fb, int fb_width, int fb_height, float dt) {
    if (!b->is_editable || !b->content) return;

    int padding = 12;
    int line_numbers_width = 50;                    // space for line numbers
    int text_x = b->x + padding + line_numbers_width;
    int text_y_base = b->y + padding;

    // Recalculate content height
    if (b->content_height <= 0.0f) {
        int num_lines = 1;
        for (const char* p = b->content; *p; p++) {
            if (*p == '\n') num_lines++;
        }
        b->content_height = (float)num_lines * b->line_height;
    }

    // Clamp scroll
    float visible_h = (float)(b->h - 2 * padding);
    float max_scroll = fmaxf(0.0f, b->content_height - visible_h);
    b->scroll_offset = fmaxf(0.0f, fminf(b->scroll_offset, max_scroll));

    int draw_y = text_y_base - (int)b->scroll_offset;

    // 1. Line Numbers
    draw_line_numbers(b, fb, fb_width, fb_height, text_x, draw_y);

    // 2. Main text
    draw_multiline_text(fb, fb_width, fb_height,
                        text_x, draw_y,
                        b->content, 0xFFFFFFFF, 1, b->line_height);

    // 3. Selection Highlighting (per-line accurate)
    draw_selection_highlighting(b, fb, fb_width, fb_height, text_x, draw_y);

    // 4. Blinking Cursor
    static float blink_timer = 0.0f;
    blink_timer += dt;
    if (((int)(blink_timer * 3) % 2) == 0) {
        int cursor_line = 0;
        int cursor_col = 0;
        int idx = 0;
        for (const char* p = b->content; *p && idx < b->cursor_pos; p++, idx++) {
            if (*p == '\n') {
                cursor_line++;
                cursor_col = 0;
            } else {
                cursor_col++;
            }
        }

        int cursor_screen_x = text_x + cursor_col * 8;
        int cursor_screen_y = draw_y + cursor_line * b->line_height;

        draw_rect(fb, fb_width, fb_height,
                  cursor_screen_x, cursor_screen_y, 2, b->line_height, 0xFFFFAA00);
    }
}

void draw_scrollbar(UI_Button* b, uint32_t* fb, int fb_width, int fb_height) {
    if (!b->is_scrollable || b->content_height <= b->h) return;

    int padding = 12;
    float visible_height = (float)(b->h - 2 * padding);
    float scroll_ratio = visible_height / b->content_height;
    float thumb_height = fmaxf(30.0f, visible_height * scroll_ratio);
    float max_scroll = fmaxf(0.0f, b->content_height - visible_height);
    float progress = (max_scroll > 0) ? (b->scroll_offset / max_scroll) : 0.0f;

    int scrollbar_x = b->x + b->w - 14;
    int scrollbar_y = b->y + padding + (int)(progress * (visible_height - thumb_height));

    draw_rect(fb, fb_width, fb_height, scrollbar_x, b->y + padding, 8, (int)visible_height, 0x30FFFFFF);
    draw_rect(fb, fb_width, fb_height, scrollbar_x, scrollbar_y, 8, (int)thumb_height, 0xCCFFFFFF);
}

void ui_draw(UI_Context* ctx, uint32_t* fb, int fb_width, int fb_height, float dt)
{
    if (!ctx->cursor_captured) return;

    memset(fb, 0, (size_t)fb_width * fb_height * sizeof(uint32_t));

    for (int i = 0; i < ctx->button_count; i++) {
        UI_Button* b = &ctx->buttons[i];

        // Background
        uint32_t panel_color = (b->target_value && ctx->button_held_last_frame[i])
                               ? 0xFF555555 : 0xB4000000;
        draw_rect(fb, fb_width, fb_height, b->x, b->y, b->w, b->h, panel_color);

        // Borders
        uint32_t border = 0x80FFFFFF;
        draw_rect(fb, fb_width, fb_height, b->x, b->y, b->w, 2, border);
        draw_rect(fb, fb_width, fb_height, b->x, b->y + b->h - 2, b->w, 2, border);
        draw_rect(fb, fb_width, fb_height, b->x, b->y, 2, b->h, border);
        draw_rect(fb, fb_width, fb_height, b->x + b->w - 2, b->y, 2, b->h, border);

        // Tuner split line
        if (b->target_value) {
            int mid = b->x + b->w / 2;
            draw_rect(fb, fb_width, fb_height, mid, b->y + 4, 2, b->h - 8, 0x00000000);
        }

        // Draw editor if it's an editable text area
        if (b->is_editable && b->content) {
            draw_editor(b, fb, fb_width, fb_height, dt);
            draw_scrollbar(b, fb, fb_width, fb_height);
        }
        // Normal scrollable or static text button
        else if (b->content && b->content[0]) {
            int padding = 12;
            int scale = 1;
            int line_h = 8 * scale + 8;
            int tx = b->x + padding;
            int ty = b->y + padding;

            if (b->is_scrollable) {
                // Calculate total content height (only once per frame is fine)
                if (b->content_height == 0.0f) {
                    int num_lines = 1;
                    for (const char* p = b->content; *p; p++) {
                        if (*p == '\n') num_lines++;
                    }
                    b->content_height = (float)num_lines * line_h;
                }
                if (b->is_scrollable && b->content_height > 0.0f) {
                    float visible_height = (float)(b->h - 24);           // inner area
                    float scroll_ratio = visible_height / b->content_height;
                    float thumb_height = fmaxf(30.0f, visible_height * scroll_ratio);

                    float max_scroll = fmaxf(0.0f, b->content_height - visible_height);
                    float scroll_progress = (max_scroll > 0.0f) ? (b->scroll_offset / max_scroll) : 0.0f;

                    int scrollbar_x = b->x + b->w - 12;
                    int scrollbar_y = b->y + 12 + (int)(scroll_progress * (visible_height - thumb_height));

                    // Scrollbar background
                    draw_rect(fb, fb_width, fb_height, scrollbar_x, b->y + 12, 6, (int)visible_height, 0x40FFFFFF);

                    // Scroll thumb
                    draw_rect(fb, fb_width, fb_height, scrollbar_x, scrollbar_y, 6, (int)thumb_height, 0xCCFFFFFF);
                }

                // Clamp scroll
                float max_scroll = fmaxf(0.0f, b->content_height - (b->h - 2*padding));
                if (b->scroll_offset > max_scroll) b->scroll_offset = max_scroll;
                if (b->scroll_offset < 0) b->scroll_offset = 0;
                
                if (b->content_height == 0.0f) {
                    int n = 1;
                    for (const char* p = b->content; *p; p++) if (*p == '\n') n++;
                    b->content_height = (float)n * line_h;
                }
                // Draw with scroll offset
                draw_multiline_text(fb, fb_width, fb_height,
                                    tx, ty - (int)b->scroll_offset,
                                    b->content, 0xFFFFFFFF, scale, line_h);
                draw_scrollbar(b, fb, fb_width, fb_height);
            } 
            else {
                // Normal non-scrollable button text
                draw_multiline_text(fb, fb_width, fb_height,
                                    tx, ty, b->content,
                                    0xFFFFFFFF, scale, line_h);
            }
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

// ====================== SELECTION & LINE NUMBERS ======================

// Draw accurate per-line selection highlighting
void draw_selection_highlighting(UI_Button* b, uint32_t* fb, int fb_width, int fb_height, int text_x, int draw_y) {
    if (b->selection_start == -1 || !b->content) return;

    int start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
    int end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;
    if (start == end) return;

    int line_height = b->line_height;
    int char_width = 8;                     // your font width

    const char* p = b->content;
    int current_line = 0;
    int char_idx = 0;
    int line_start_x = text_x;

    while (*p && char_idx < end) {
        if (char_idx >= start) {
            // Start of selection on this line
            int sel_start_col = (char_idx == start) ? 0 : 0; // will be refined
            int sel_end_col = 0;

            // Find how many chars to highlight on this line
            const char* line_start = p;
            while (*p && *p != '\n' && char_idx < end) {
                p++;
                char_idx++;
            }
            sel_end_col = (int)(p - line_start);

            if (char_idx > start) {
                int sel_from = (char_idx - sel_end_col > start) ? 0 : (start - (char_idx - sel_end_col));
                int sel_to = (char_idx >= end) ? (end - (char_idx - sel_end_col)) : sel_end_col;

                int highlight_x = text_x + sel_from * char_width;
                int highlight_w = (sel_to - sel_from) * char_width;

                draw_rect(fb, fb_width, fb_height,
                          highlight_x,
                          draw_y + current_line * line_height,
                          highlight_w,
                          line_height,
                          0x4080C0FF);   // nice blue selection color
            }
        }

        if (*p == '\n') {
            p++;
            current_line++;
            char_idx++;
        } else if (*p) {
            p++;
            char_idx++;
        }
    }
}

// Draw line numbers
void draw_line_numbers(UI_Button* b, uint32_t* fb, int fb_width, int fb_height, int text_x, int draw_y) {
    if (!b->content) return;

    int line_height = b->line_height;
    int num_lines = 1;
    for (const char* p = b->content; *p; p++) {
        if (*p == '\n') num_lines++;
    }

    int line_num_x = b->x + 8;                    // left margin
    int current_line = 1;

    char num_buf[16];

    for (int i = 0; i < num_lines; i++) {
        snprintf(num_buf, sizeof(num_buf), "%3d", current_line);

        draw_text(fb, fb_width, fb_height,
                  num_buf,
                  line_num_x,
                  draw_y + i * line_height,
                  0xFF888888, 1);   // gray line numbers

        current_line++;
    }
}