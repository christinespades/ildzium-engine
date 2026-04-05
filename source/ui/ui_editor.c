#include "ui_editor.h"
#include <GLFW/glfw3.h>
#include <ctype.h>    // isalnum
#include <stdlib.h>   // malloc, free
#include <string.h>   // strlen, strcpy

extern GLFWwindow* g_window; 

void ensure_capacity(UI_Button* b, size_t needed) {
    if (b->content_capacity >= needed) return;
    size_t new_cap = needed + 2048; // grow generously
    char* new_buf = (char*)realloc(b->content, new_cap);
    if (new_buf) {
        b->content = new_buf;
        b->content_capacity = new_cap;
    }
}

void insert_char_at_cursor(UI_Button* b, char c) {
    if (!b->is_editable || !b->content) return;

    // Delete selection first if any
    if (b->selection_start != -1) {
        int start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
        int end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;
        memmove(b->content + start, b->content + end, strlen(b->content + end) + 1);
        b->cursor_pos = start;
        b->selection_start = b->selection_end = -1;
    }

    size_t len = strlen(b->content);
    ensure_capacity(b, len + 2);

    memmove(b->content + b->cursor_pos + 1, b->content + b->cursor_pos, len - b->cursor_pos + 1);
    b->content[b->cursor_pos] = c;
    b->cursor_pos++;
    b->content_height = 0.0f; // invalidate
}

void delete_char_before_cursor(UI_Button* b) {
    if (!b->is_editable || b->cursor_pos <= 0) return;
    if (b->selection_start != -1) {
        int start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
        int end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;
        memmove(b->content + start, b->content + end, strlen(b->content + end) + 1);
        b->cursor_pos = start;
        b->selection_start = b->selection_end = -1;
    } else {
        memmove(b->content + b->cursor_pos - 1, b->content + b->cursor_pos, strlen(b->content + b->cursor_pos) + 1);
        b->cursor_pos--;
    }
    b->content_height = 0.0f;
}

void delete_char_at_cursor(UI_Button* b) {
    if (!b->is_editable) return;
    size_t len = strlen(b->content);
    if (b->cursor_pos >= len) return;

    if (b->selection_start != -1) {
        int start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
        int end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;
        memmove(b->content + start, b->content + end, strlen(b->content + end) + 1);
        b->cursor_pos = start;
        b->selection_start = b->selection_end = -1;
    } else {
        memmove(b->content + b->cursor_pos, b->content + b->cursor_pos + 1, len - b->cursor_pos);
    }
    b->content_height = 0.0f;
}

void move_cursor_left(UI_Button* b, int steps) {
    if (b->cursor_pos > 0) b->cursor_pos = (b->cursor_pos > steps) ? b->cursor_pos - steps : 0;
}

void move_cursor_right(UI_Button* b, int steps) {
    size_t len = strlen(b->content);
    if (b->cursor_pos < len) b->cursor_pos = (b->cursor_pos + steps < len) ? b->cursor_pos + steps : (int)len;
}

void move_to_home(UI_Button* b) { b->cursor_pos = 0; }
void move_to_end(UI_Button* b)  { b->cursor_pos = (int)strlen(b->content); }

int get_text_length(UI_Button* b) {
    return b->content ? (int)strlen(b->content) : 0;
}

// ====================== CLIPBOARD ======================

void copy_selection_to_clipboard(UI_Button* b) {
    if (b->selection_start == -1 || !b->content) return;

    int start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
    int end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;

    if (start == end) return;

    char* selected = (char*)malloc(end - start + 1);
    strncpy(selected, b->content + start, end - start);
    selected[end - start] = '\0';

    glfwSetClipboardString(g_window, selected);
    free(selected);
}

void paste_from_clipboard(UI_Button* b) {
    if (!b->is_editable || !b->content) return;

    const char* clipboard = glfwGetClipboardString(g_window);
    if (!clipboard || !*clipboard) return;

    // Delete current selection first
    if (b->selection_start != -1) {
        int start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
        int end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;
        memmove(b->content + start, b->content + end, strlen(b->content + end) + 1);
        b->cursor_pos = start;
        b->selection_start = b->selection_end = -1;
    }

    size_t clip_len = strlen(clipboard);
    size_t cur_len = strlen(b->content);
    ensure_capacity(b, cur_len + clip_len + 1);

    memmove(b->content + b->cursor_pos + clip_len, b->content + b->cursor_pos, cur_len - b->cursor_pos + 1);
    memcpy(b->content + b->cursor_pos, clipboard, clip_len);

    b->cursor_pos += (int)clip_len;
    b->content_height = 0.0f; // invalidate
}

// ====================== MOUSE TO CURSOR ======================

int get_char_index_from_mouse(UI_Button* b, int mouse_x, int mouse_y) {
    if (!b->content) return 0;

    int padding = 12;
    int text_x = b->x + padding;
    int text_y = b->y + padding;

    int rel_x = mouse_x - text_x;
    int rel_y = mouse_y - text_y + (int)b->scroll_offset;

    if (rel_x < 0) rel_x = 0;
    if (rel_y < 0) rel_y = 0;

    int line_height = b->line_height;
    int target_line = rel_y / line_height;

    // Find start of target line
    const char* p = b->content;
    int current_line = 0;
    int char_index = 0;

    while (*p && current_line < target_line) {
        if (*p == '\n') current_line++;
        p++;
        char_index++;
    }

    // Now walk horizontally on this line
    int x_pos = 0;
    while (*p && *p != '\n') {
        if (x_pos + 8 > rel_x) break;   // 8 = char width (your font)
        x_pos += 8;
        p++;
        char_index++;
    }

    return char_index;
}

void select_word_at_position(UI_Button* b, int click_pos) {
    if (!b->content || click_pos < 0) return;

    int len = (int)strlen(b->content);
    if (click_pos > len) click_pos = len;

    // Find start of word (go left until non-word char)
    int start = click_pos;
    while (start > 0 && isalnum(b->content[start-1]) || b->content[start-1] == '_') {
        start--;
    }

    // Find end of word (go right)
    int end = click_pos;
    while (end < len && (isalnum(b->content[end]) || b->content[end] == '_')) {
        end++;
    }

    b->selection_start = start;
    b->selection_end = end;
    b->cursor_pos = end;
}