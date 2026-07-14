#include "pch.h"
#include "ui/ui_editor_string.h"

void insert_char_at_cursor(UI_Button* b, char c) {
    if (!b->is_editable || !b->editable_content) return;
    if (b->is_typing)
    {
        // tuner editor
        LOGI("inserting char for tuner editor");

        if(!(isdigit((unsigned char)c) || c=='.' || c=='-' || c=='+'))
            return;

        if(b->selection_start!=-1)
        {
            int a=min(b->selection_start,b->selection_end);
            int b2=max(b->selection_start,b->selection_end);

            memmove(
                b->editable_content+a,
                b->editable_content+b2,
                strlen(b->editable_content+b2)+1);

            b->cursor_pos=a;
            b->selection_start=-1;
            b->selection_end=-1;
        }

        size_t len=strlen(b->editable_content);

        if(len>=sizeof(b->editable_content)-1)
            return;

        memmove(
            b->editable_content+b->cursor_pos+1,
            b->editable_content+b->cursor_pos,
            len-b->cursor_pos+1);

        b->editable_content[b->cursor_pos]=c;
        b->cursor_pos++;
        return;
    }

    // regular text editor
    push_undo_state(b);

    // Delete selection first if any
    if (b->selection_start != -1) {
        int start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
        int end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;
        memmove(b->editable_content + start, b->editable_content + end, strlen(b->editable_content + end) + 1);
        b->cursor_pos = start;
        b->selection_start = b->selection_end = -1;
    }

    size_t len = strlen(b->editable_content);
    ensure_capacity(b, len + 2);

    memmove(b->editable_content + b->cursor_pos + 1, b->editable_content + b->cursor_pos, len - b->cursor_pos + 1);
    b->editable_content[b->cursor_pos] = c;
    b->cursor_pos++;
    b->content_height = 0.0f; // invalidate
}

void insert_string_at_cursor(UI_Button* b, const char* text)
{
    if (!b->is_editable || !b->editable_content || !text) return;

    push_undo_state(b);

    // handle selection deletion first
    if (b->selection_start != -1) {
        int start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
        int end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;

        memmove(b->editable_content + start,
                b->editable_content + end,
                strlen(b->editable_content + end) + 1);

        b->cursor_pos = start;
        b->selection_start = b->selection_end = -1;
    }

    size_t len = strlen(b->editable_content);
    size_t tlen = strlen(text);

    ensure_capacity(b, len + tlen + 1);

    memmove(b->editable_content + b->cursor_pos + tlen,
            b->editable_content + b->cursor_pos,
            len - b->cursor_pos + 1);

    memcpy(b->editable_content + b->cursor_pos, text, tlen);

    b->cursor_pos += tlen;

    b->content_height = 0.0f;
}

void delete_char_before_cursor(UI_Button* b) {
    if (!b->is_editable || b->cursor_pos <= 0) return;

    push_undo_state(b);

    if (b->selection_start != -1) {
        int start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
        int end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;
        memmove(b->editable_content + start, b->editable_content + end, strlen(b->editable_content + end) + 1);
        b->cursor_pos = start;
        b->selection_start = b->selection_end = -1;
    } else {
        memmove(b->editable_content + b->cursor_pos - 1, b->editable_content + b->cursor_pos, strlen(b->editable_content + b->cursor_pos) + 1);
        b->cursor_pos--;
    }
    b->content_height = 0.0f;
}

void delete_char_at_cursor(UI_Button* b) {
    if (!b->is_editable) return;
    size_t len = strlen(b->editable_content);
    if (b->cursor_pos >= len) return;

    push_undo_state(b);

    if (b->selection_start != -1) {
        int start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
        int end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;
        memmove(b->editable_content + start, b->editable_content + end, strlen(b->editable_content + end) + 1);
        b->cursor_pos = start;
        b->selection_start = b->selection_end = -1;
    } else {
        memmove(b->editable_content + b->cursor_pos, b->editable_content + b->cursor_pos + 1, len - b->cursor_pos);
    }
    b->content_height = 0.0f;
}