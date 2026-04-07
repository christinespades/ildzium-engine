#include "core/debug.h"
#include "ui/ui_draw.h"
#include "ui/ui_draw_info.h"
#include "ui/ui_draw_text_rect.h"
#include "ui/ui.h"
#include "ui/ui_params.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// ====================== DRAW HELPERS ======================
void draw_editor(UI_Button* b, uint32_t* fb, int fb_width, int fb_height, float dt)
{
    if (!b->is_editable || !b->editable_content || b->editable_content[0] == '\0')
        return;

    // Safety defaults
    if (b->line_height <= 0)
        b->line_height = FONT_HEIGHT + 8;   // typical value

    // === Recalculate content height only when needed (more accurate) ===
    if (b->content_height <= 0.0f)
    {
        int num_lines = 1;
        for (const char* p = b->editable_content; *p; ++p)
        {
            if (*p == '\n')
                num_lines++;
        }
        b->content_height = (float)num_lines * b->line_height;
    }

    // Clamp cursor
    size_t text_len = strlen(b->editable_content);
    if (b->cursor_pos < 0)
        b->cursor_pos = 0;
    if ((size_t)b->cursor_pos > text_len)
        b->cursor_pos = (int)text_len;

    const char* text = b->editable_content;
    int padding = 12;
    int text_x = b->x + padding + LINE_NUMBERS_WIDTH;   // make sure LINE_NUMBERS_WIDTH is defined
    int text_y_base = b->y + padding;

    // Clamp scroll offset
    float visible_h = (float)(b->h - 2 * padding);
    float max_scroll = fmaxf(0.0f, b->content_height - visible_h);
    b->scroll_offset = fmaxf(0.0f, fminf(b->scroll_offset, max_scroll));

    int draw_y = text_y_base - (int)b->scroll_offset;

    // 1. Line Numbers
    draw_line_numbers(b, fb, fb_width, fb_height, text_x - LINE_NUMBERS_WIDTH, draw_y);

    // 2. Main text
    draw_multiline_text(fb, fb_width, fb_height,
                        text_x, draw_y,
                        text, 0xFFFFFFFF, 1, b->line_height);

    // 3. Selection Highlighting
    if (b->selection_start != -1)
        draw_selection_highlighting(b, fb, fb_width, fb_height, text_x, draw_y);

    // 4. Blinking Cursor
    static float blink_timer = 0.0f;
    blink_timer += dt;
    if (((int)(blink_timer * 3.0f) % 2) == 0)
    {
        // Calculate cursor line and column safely
        int cursor_line = 0;
        int cursor_col = 0;
        int idx = 0;

        for (const char* p = text; *p && idx < b->cursor_pos; ++p, ++idx)
        {
            if (*p == '\n')
            {
                cursor_line++;
                cursor_col = 0;
            }
            else
            {
                cursor_col++;
            }
        }

        int cursor_screen_x = text_x + cursor_col * FONT_WIDTH;
        int cursor_screen_y = draw_y + cursor_line * b->line_height;

        // Extra safety: only draw cursor if it's inside the button area
        if (cursor_screen_y + b->line_height > b->y && cursor_screen_y < b->y + b->h)
        {
            draw_rect(fb, fb_width, fb_height,
                      cursor_screen_x, cursor_screen_y,
                      2, b->line_height,
                      0xFFFFAA00);
        }
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
    if (!ctx || !fb || fb_width <= 0 || fb_height <= 0 || !ctx->cursor_captured)
        return;

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

            // Current value
            char val[32];
            // ensure pointer is valid
            float value = *b->target_value; 
            snprintf(val, sizeof(val), "%.3f", value);
            draw_text(fb, fb_width, fb_height, val, 
                      b->x + 20, b->y + b->h - 16, 0xFFFFAA00, 2);
        }

        // Draw editor if it's an editable text area
        if (b->is_editable && b->editable_content) {
            draw_editor(b, fb, fb_width, fb_height, dt);
            draw_scrollbar(b, fb, fb_width, fb_height);
        }
        // Normal scrollable or static text button
        else if (b->content && b->content[0]) {
            int padding = 12;
            int scale = 1;
            int line_h = FONT_HEIGHT * scale + 8;
            int tx = b->x + padding;
            int ty = b->y + padding;

            if (b->is_scrollable) {
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
    }

    ui_draw_info(fb, fb_width, fb_height, dt); // fps, ui focus, debug, etc
}

// ====================== SELECTION & LINE NUMBERS ======================
// Draw accurate per-line selection highlighting
// For both read-only text (content) and mutable text (editable_content)
// Draw accurate per-character selection highlighting (black)
void draw_selection_highlighting(UI_Button* b, uint32_t* fb, int fb_width, int fb_height,
                                 int text_x, int draw_y)
{
    const char* p = b->is_editable ? b->editable_content : b->content;
    if (b->selection_start == -1 || !p) return;

    int start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
    int end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;
    if (start == end) return;

    int line_height = b->line_height;
    if (line_height <= 0) line_height = FONT_HEIGHT + 4; // fallback

    int current_line = 0;
    int char_idx = 0;
    const char* line_start = p;

    while (*p && char_idx < end)
    {
        if (char_idx >= start)
        {
            // Find how many characters are selected on this line
            int line_sel_start = (start > char_idx) ? (start - char_idx) : 0;
            int line_sel_end = 0;

            const char* q = p;
            int temp_idx = char_idx;
            while (*q && *q != '\n' && temp_idx < end)
            {
                q++;
                temp_idx++;
                line_sel_end++;
            }

            int sel_from = line_sel_start;
            int sel_to   = (temp_idx >= end) ? (end - char_idx) : line_sel_end;

            if (sel_to > sel_from)
            {
                int highlight_x = text_x + sel_from * FONT_WIDTH;
                int highlight_w = (sel_to - sel_from) * FONT_WIDTH;
                int highlight_y = draw_y + current_line * line_height;

                // Black selection with good opacity (text stays readable)
                draw_rect(fb, fb_width, fb_height,
                          highlight_x, highlight_y,
                          highlight_w, line_height,
                          0x000000CC);   // <-- changed to black
            }
        }

        // Advance to next line
        if (*p == '\n')
        {
            p++;
            char_idx++;
            current_line++;
            line_start = p;
        }
        else if (*p)
        {
            p++;
            char_idx++;
        }
    }
}

// Draw line numbers
void draw_line_numbers(UI_Button* b, uint32_t* fb, int fb_width, int fb_height,
                       int text_x, int draw_y)
{
    if (!b->editable_content) return;

    int line_height = b->line_height;
    int num_lines = 1;
    for (const char* p = b->editable_content; *p; p++) {
        if (*p == '\n') num_lines++;
    }

    int line_num_x = b->x + 8;
    int current_line = 1;
    char num_buf[16];

    for (int i = 0; i < num_lines; i++)
    {
        int y_pos = draw_y + i * line_height;

        // Only draw if the line number is at least partially visible
        if (y_pos + FONT_HEIGHT > 0 && y_pos < fb_height)
        {
            snprintf(num_buf, sizeof(num_buf), "%3d", current_line);
            draw_text(fb, fb_width, fb_height,
                      num_buf,
                      line_num_x,
                      y_pos,
                      0xFF888888, 1);
        }
        current_line++;
    }
}