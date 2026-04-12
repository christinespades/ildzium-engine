#include "pch.h"
#include "ui_editor.h"

#ifndef __EMSCRIPTEN__
    extern GLFWwindow* g_window; 
#endif

static void get_cursor_line_info(const char* text, int cursor,
                                 int* out_line_start,
                                 int* out_col)
{
    int col = 0;
    int line_start = 0;

    for (int i = 0; i < cursor && text[i]; i++) {
        if (text[i] == '\n') {
            line_start = i + 1;
            col = 0;
        } else {
            col++;
        }
    }

    *out_line_start = line_start;
    *out_col = col;
}

void ensure_capacity(UI_Button* b, size_t needed) {
    if (b->content_capacity >= needed) return;
    size_t new_cap = needed + 2048; // grow generously
    char* new_buf = (char*)realloc(b->editable_content, new_cap);
    if (new_buf) {
        b->editable_content = new_buf;
        b->content_capacity = new_cap;
    }
}

void move_cursor_left(UI_Button* b, int steps) {
    if (b->cursor_pos > 0) b->cursor_pos = (b->cursor_pos > steps) ? b->cursor_pos - steps : 0;
}

void move_cursor_right(UI_Button* b, int steps) {
    size_t len = strlen(b->editable_content);
    if (b->cursor_pos < len) b->cursor_pos = (b->cursor_pos + steps < len) ? b->cursor_pos + steps : (int)len;
}

void move_cursor_vertical(UI_Button* b, int direction)
{
    const char* text = b->editable_content;
    if (!text) return;

    int line_start, col;
    get_cursor_line_info(text, b->cursor_pos, &line_start, &col);

    if (direction < 0) {
        // UP
        if (line_start == 0) return;

        int prev_line_end = line_start - 1;
        int prev_line_start = prev_line_end;

        while (prev_line_start > 0 && text[prev_line_start - 1] != '\n')
            prev_line_start--;

        int prev_len = prev_line_end - prev_line_start;
        int new_col = col < prev_len ? col : prev_len;

        b->cursor_pos = prev_line_start + new_col;
    } else {
        // DOWN
        int i = line_start;
        while (text[i] && text[i] != '\n') i++;
        if (!text[i]) return; // no next line

        int next_line_start = i + 1;

        int next_len = 0;
        while (text[next_line_start + next_len] &&
               text[next_line_start + next_len] != '\n')
            next_len++;

        int new_col = col < next_len ? col : next_len;

        b->cursor_pos = next_line_start + new_col;
    }
}

void move_to_home(UI_Button* b) { b->cursor_pos = 0; }
void move_to_end(UI_Button* b)  { b->cursor_pos = (int)strlen(b->editable_content); }

int get_text_length(UI_Button* b) {
    return b->editable_content ? (int)strlen(b->editable_content) : 0;
}

// ====================== MOUSE TO CURSOR ======================
int get_char_index_from_mouse(UI_Button* b, int mouse_x, int mouse_y)
{
    if ((!b->content && !b->editable_content) || b->line_height <= 0) return 0;

    int padding = 12;
    int text_x = b->x + padding;
    if (b->is_editable) text_x += EDITOR_LINE_NUMBERS_WIDTH;

    int text_y = b->y + padding;
    int rel_x = mouse_x - text_x;
    int rel_y = mouse_y - text_y + (int)b->scroll_offset;

    if (rel_x < 0) rel_x = 0;
    if (rel_y < 0) rel_y = 0;

    int target_line = rel_y / b->line_height;

    const char* p = b->is_editable ? b->editable_content : b->content;
    int char_index = 0;
    int current_line = 0;

    // Skip to target line
    while (*p && current_line < target_line)
    {
        if (*p == '\n') current_line++;
        p++;
        char_index++;
    }

    // Now walk horizontally on the line (more forgiving at end of line)
    int x_pos = 0;
    while (*p && *p != '\n')
    {
        if (x_pos + FONT_WIDTH / 2 > rel_x) break;   // better threshold (half char width)
        x_pos += FONT_WIDTH;
        p++;
        char_index++;
    }

    // If clicked past the end of the line, go to end of line instead of next line
    return char_index;
}

void select_word_at_position(UI_Button* b, int click_pos) {
    const char* text = b->is_editable ? b->editable_content : b->content;
    if (!text || click_pos < 0) return;

    int len = (int)strlen(text);
    if (click_pos > len) click_pos = len;

    // Find start of word (go left until non-word char)
    int start = click_pos;
    while (start > 0 && is_word_char(text[start - 1])) {
        start--;
    }

    // Find end of word (go right)
    int end = click_pos;
    while (end < len && is_word_char(text[end])) {
        end++;
    }

    // If click is on whitespace → select nothing or just move cursor
    // If click is at end of text → clamp to last word
    if (!is_word_char(text[click_pos])) {
        b->selection_start = b->selection_end = click_pos;
        b->cursor_pos = click_pos;
        return;
    }

    b->selection_start = start;
    b->selection_end = end;
    b->cursor_pos = end;
}