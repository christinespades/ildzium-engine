#include "pch.h"
#include "ui/ui_draw.h"

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
    int text_x = b->x + padding + EDITOR_LINE_NUMBERS_WIDTH;
    int text_y_base = b->y + padding;

    // Clamp scroll offset
    float visible_h = (float)(b->h - 2 * padding);
    float max_scroll = fmaxf(0.0f, b->content_height - visible_h);
    b->scroll_offset = fmaxf(0.0f, fminf(b->scroll_offset, max_scroll));

    int draw_y = text_y_base - (int)b->scroll_offset;

    // 1. Line Numbers
    draw_line_numbers(b, fb, fb_width, fb_height, text_x - EDITOR_LINE_NUMBERS_WIDTH, draw_y);

    // Selection Highlighting
    if (b->selection_start != -1)
        draw_selection_highlighting(b, fb, fb_width, fb_height, text_x, draw_y);

    // Main text
    draw_multiline_text(fb, fb_width, fb_height,
                        text_x, draw_y,
                        text, COLOR_EDITOR_MAIN_TEXT, 1, b->line_height, b);

    // 4. Blinking Caret
    static float blink_timer = 0.0f;
    blink_timer += dt;
    if (((int)(blink_timer * 3.0f) % 2) == 0)
    {
        // Calculate caret line and column safely
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

        // Extra safety: only draw caret if it's inside the button area
        if (cursor_screen_y + b->line_height > b->y && cursor_screen_y < b->y + b->h)
        {
            draw_rect(fb, fb_width, fb_height,
                      cursor_screen_x, cursor_screen_y,
                      2, b->line_height,
                      COLOR_EDITOR_CARET);
        }
    }
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
                               ? COLOR_TUNER_PANEL_ACTIVE : COLOR_TUNER_PANEL_IDLE;
        draw_rect(fb, fb_width, fb_height, b->x, b->y, b->w, b->h, panel_color);

        // Borders
        uint32_t border = COLOR_BORDER;
        draw_rect(fb, fb_width, fb_height, b->x, b->y, b->w, 2, border);
        draw_rect(fb, fb_width, fb_height, b->x, b->y + b->h - 2, b->w, 2, border);
        draw_rect(fb, fb_width, fb_height, b->x, b->y, 2, b->h, border);
        draw_rect(fb, fb_width, fb_height, b->x + b->w - 2, b->y, 2, b->h, border);

        // Tuner split line
        if (b->target_value) {
            int mid = b->x + b->w / 2;
            draw_rect(fb, fb_width, fb_height, mid, b->y + 4, 2, b->h - 8, COLOR_TUNER_SPLIT_LINE);

            // Current value
            char val[32];
            // ensure pointer is valid
            float value = *b->target_value; 
            snprintf(val, sizeof(val), "%.3f", value);
            draw_text(fb, fb_width, fb_height, val, 
                      b->x + 20, b->y + b->h - 16, COLOR_TUNER_VALUE_TEXT, 2, b);
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
                                    b->content, COLOR_MAIN_TEXT, scale, line_h, b);
                draw_scrollbar(b, fb, fb_width, fb_height);
            } 
            else {
                // Normal non-scrollable button text
                draw_multiline_text(fb, fb_width, fb_height,
                                    tx, ty, b->content,
                                    COLOR_MAIN_TEXT, scale, line_h, b);
            }
        }
    }

    ui_draw_info(fb, fb_width, fb_height, dt); // fps, ui focus, debug, etc
}

// ====================== SELECTION & LINE NUMBERS ======================
// Draw accurate per-line selection highlighting
// For both read-only text (content) and mutable text (editable_content)
// Draw accurate per-character selection highlighting (black)
// Corrected: draws the highlight exactly where the selected characters are (not at line start)
// FIXED: Highlights exactly the selected range (word, multiple words, lines, etc.)
void draw_selection_highlighting(UI_Button* b, uint32_t* fb, int fb_width, int fb_height,
                                 int text_x, int draw_y)
{
    const char* text = b->is_editable ? b->editable_content : b->content;
    if (b->selection_start == -1 || !text) return;

    int sel_start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
    int sel_end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;
    if (sel_start == sel_end) return;

    int line_height = b->line_height > 0 ? b->line_height : (FONT_HEIGHT + 4);

    int current_line = 0;
    int char_idx = 0;
    const char* p = text;

    while (*p && char_idx < sel_end)
    {
        int line_start_idx = char_idx;   // remember where this line begins

        // Skip to end of line or end of selection
        while (*p && *p != '\n' && char_idx < sel_end)
        {
            if (char_idx >= sel_start)
            {
                // We are inside the selection on this line
                int from = (sel_start > line_start_idx) ? (sel_start - line_start_idx) : 0;
                int to   = (sel_end   > char_idx)      ? (char_idx - line_start_idx + 1) : (char_idx - line_start_idx);

                // Only draw if there's something to highlight on this line
                if (to > from)
                {
                    int highlight_x = text_x + from * FONT_WIDTH;
                    int highlight_w = (to - from) * FONT_WIDTH;
                    int highlight_y = draw_y + current_line * line_height;

                    draw_rect(fb, fb_width, fb_height,
                              highlight_x, highlight_y,
                              highlight_w, line_height,
                              COLOR_TEXT_HIGHLIGHT);
                }
            }

            p++;
            char_idx++;
        }

        // Handle newline
        if (*p == '\n')
        {
            p++;
            char_idx++;
            current_line++;
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
                      COLOR_EDITOR_LINE_NUMBERS, 1, b);
        }
        current_line++;
    }
}