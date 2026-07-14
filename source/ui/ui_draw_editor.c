#include "pch.h"
#include "ui_draw_editor.h"

void draw_editor(UI_Button* b)
{
    if (!b->is_editable || !b->editable_content || b->editable_content[0] == '\0')
        return;
    int padding = 12;
    if (b->target_value != NULL) {
        LOGI("drawing editor for tuner %s", b->content);
        draw_panel(
            g_ui_fb,
            g_width,
            g_height,
            b->x,
            b->y,
            b->w,
            b->h,
            get_param_color(PARAM_UI_COLOR_TUNER_PANEL_ACTIVE),
            get_param_color(PARAM_UI_COLOR_TUNER_BORDER_INNER));

        int text_x = b->x + padding;
        int text_y = b->y + (b->h - get_param_float(PARAM_UI_FONT_HEIGHT)) / 2;

        if (b->selection_start != -1)
        {
            int start = b->selection_start < b->selection_end ?
                b->selection_start :
                b->selection_end;

            int end = b->selection_start > b->selection_end ?
                b->selection_start :
                b->selection_end;

            for (int i = start; i < end; i++)
            {
                draw_rect(
                    g_ui_fb,
                    g_width,
                    g_height,
                    text_x + i * get_param_float(PARAM_UI_FONT_WIDTH),
                    text_y,
                    get_param_float(PARAM_UI_FONT_WIDTH),
                    get_param_float(PARAM_UI_FONT_HEIGHT),
                    get_param_color(PARAM_UI_COLOR_TUNER_BORDER_OUTER));
            }
        }

        draw_text(
            g_ui_fb,
            g_width,
            g_height,
            b->editable_content,
            text_x,
            text_y,
            get_param_color(PARAM_UI_COLOR_EDITOR_MAIN_TEXT),
            1,
            NULL);

        if ((g_current_frame / 3) % 2 == 0)
        {
            draw_rect(
                g_ui_fb,
                g_width,
                g_height,
                text_x + b->cursor_pos * get_param_float(PARAM_UI_FONT_WIDTH),
                text_y,
                2,
                get_param_float(PARAM_UI_FONT_HEIGHT),
                get_param_color(PARAM_UI_COLOR_EDITOR_CARET));
        }
        return;
    }
    //LOGI("drawing editor for button %s", b->content);
    // Safety defaults
    if (b->line_height <= 0)
        b->line_height = get_param_float(PARAM_UI_FONT_HEIGHT) + 8;   // typical value

    const char* text = b->editable_content;
    int line_numbers_w = get_param_float(PARAM_UI_EDITOR_LINE_NUMBERS_WIDTH);
    int text_x_base = b->x + padding + line_numbers_w;
    int text_y_base = b->y + padding;

    // === Recalculate content height & width only when needed ===
    if (b->content_height <= 0.0f || b->content_width <= 0.0f)
    {
        int num_lines = 1;
        int current_line_chars = 0;
        int max_line_chars = 0;

        for (const char* p = text; *p; ++p)
        {
            if (*p == '\n')
            {
                num_lines++;
                if (current_line_chars > max_line_chars) max_line_chars = current_line_chars;
                current_line_chars = 0;
            }
            else
            {
                current_line_chars++;
            }
        }
        if (current_line_chars > max_line_chars) max_line_chars = current_line_chars;

        b->content_height = (float)num_lines * b->line_height;
        b->content_width = (float)max_line_chars * get_param_float(PARAM_UI_FONT_WIDTH);
    }

    // Clamp cursor
    size_t text_len = strlen(text);
    if (b->cursor_pos < 0)          b->cursor_pos = 0;
    if ((size_t)b->cursor_pos > text_len) b->cursor_pos = (int)text_len;

    // === Clamp vertical and horizontal scroll offsets ===
    float visible_h = (float)(b->h - 2 * padding);
    float max_scroll_y = fmaxf(0.0f, b->content_height - visible_h);
    b->scroll_offset = fmaxf(0.0f, fminf(b->scroll_offset, max_scroll_y));

    // Visible width takes the line numbers column into account
    float visible_w = (float)(b->w - 2 * padding - line_numbers_w);
    float max_scroll_x = fmaxf(0.0f, b->content_width - visible_w);
    b->scroll_offset_x = fmaxf(0.0f, fminf(b->scroll_offset_x, max_scroll_x));

    // Final shifted drawing coordinates
    int draw_x = text_x_base - (int)b->scroll_offset_x;
    int draw_y = text_y_base - (int)b->scroll_offset;

    // 1. Line Numbers (Stays statically left; only scrolls vertically)
    draw_line_numbers(b, g_ui_fb, g_width, g_height, b->x + padding, draw_y);

    // 2. Selection Highlighting (Must use shifted draw_x)
    if (b->selection_start != -1)
        draw_selection_highlighting(b, g_ui_fb, g_width, g_height, draw_x, draw_y);

    // 3. Main text (Must use shifted draw_x)
    draw_multiline_text(g_ui_fb, g_width, g_height,
                        draw_x, draw_y,
                        text, get_param_color(PARAM_UI_COLOR_EDITOR_MAIN_TEXT), 1, b->line_height, b);

    // 4. Blinking Caret (Must apply horizontal scroll)
    if (ui_button_has_focus(b)) {
        static float blink_timer = 0.0f;
        blink_timer += g_dt;
        if (((int)(blink_timer * 3.0f) % 2) == 0)
        {
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

            // Apply horizontal scroll to screen position
            int cursor_screen_x = draw_x + cursor_col * get_param_float(PARAM_UI_FONT_WIDTH);
            int cursor_screen_y = draw_y + cursor_line * b->line_height;

            // Clip caret to the text bounds area (so it doesn't leak into line numbers or margins)
            if (cursor_screen_y + b->line_height > b->y && cursor_screen_y < b->y + b->h &&
                cursor_screen_x >= text_x_base && cursor_screen_x < b->x + b->w - padding)
            {
                draw_rect(g_ui_fb, g_width, g_height,
                          cursor_screen_x, cursor_screen_y,
                          2, b->line_height,
                          get_param_color(PARAM_UI_COLOR_EDITOR_CARET));
            }
        }
    }
}

// ====================== SELECTION & LINE NUMBERS ======================
// Draw accurate per-line selection highlighting
// For both read-only text (content) and mutable text (editable_content)
// Draw accurate per-character selection highlighting where the selected characters are (not at line start) and highlights the selected range (word, multiple words, lines, etc.)
void draw_selection_highlighting(UI_Button* b, uint32_t* fb, int fb_width, int fb_height, int text_x, int draw_y)
{
    const char* text = b->is_editable ? b->editable_content : b->content;
    if (b->selection_start == -1 || !text) return;

    int sel_start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
    int sel_end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;
    if (sel_start == sel_end) return;

    int line_height = b->line_height > 0 ? b->line_height : (get_param_float(PARAM_UI_FONT_HEIGHT) + 4);
    float font_w = get_param_float(PARAM_UI_FONT_WIDTH);

    int current_line = 0;
    int char_idx = 0;
    const char* p = text;

    while (*p && char_idx < sel_end)
    {
        int line_start_idx = char_idx;
        int line_sel_start = -1;
        int line_sel_end = -1;

        // Process the current line to find the batch selection range
        while (*p && *p != '\n')
        {
            if (char_idx >= sel_start && char_idx < sel_end)
            {
                if (line_sel_start == -1) {
                    line_sel_start = char_idx - line_start_idx;
                }
                line_sel_end = char_idx - line_start_idx + 1;
            }
            p++;
            char_idx++;
        }

        // Draw ONE rectangle for this entire line's selection (if it falls in range)
        if (line_sel_start != -1)
        {
            int highlight_x = text_x + (int)(line_sel_start * font_w);
            int highlight_w = (int)((line_sel_end - line_sel_start) * font_w);
            int highlight_y = draw_y + current_line * line_height;

            draw_rect(fb, fb_width, fb_height,
                      highlight_x, highlight_y,
                      highlight_w, line_height,
                      get_param_color(PARAM_UI_COLOR_TEXT_HIGHLIGHT));
        }

        // Handle newline transition
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

    // Calculate the exact top and bottom limits based on your editor padding
    int padding = 12;
    int clip_t = b->y + padding;
    int clip_b = b->y + b->h - padding;

    for (int i = 0; i < num_lines; i++)
    {
        int y_pos = draw_y + i * line_height;

        // Tighter visibility check: Only draw if the line number sits entirely within the text zone
        if (y_pos >= clip_t && (y_pos + get_param_float(PARAM_UI_FONT_HEIGHT)) <= clip_b)
        {
            snprintf(num_buf, sizeof(num_buf), "%3d", current_line);
            draw_text(fb, fb_width, fb_height,
                      num_buf,
                      line_num_x,
                      y_pos,
                      get_param_color(PARAM_UI_COLOR_EDITOR_LINE_NUMBERS), 1, b);
        }
        current_line++;
    }
}
