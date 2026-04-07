#pragma once
#include "ui_editor.h"
#include "ui/ui_params.h"
#include <GLFW/glfw3.h>
#include <ctype.h>    // isalnum
#include <stdlib.h>   // malloc, free
#include <string.h>   // strlen, strcpy

void free_editor_state(EditorState* s) {
    if (s && s->text) free(s->text);
}

// must be called when button is destroyed
void cleanup_editor_undo(UI_Button* b) {
    if (b->undo_stack) {
        for (int i = 0; i < b->undo_count; i++)
            free_editor_state(&b->undo_stack[i]);
        free(b->undo_stack);
    }
    if (b->redo_stack) {
        for (int i = 0; i < b->redo_count; i++)
            free_editor_state(&b->redo_stack[i]);
        free(b->redo_stack);
    }
}


void copy_editor_state(EditorState* dst, const UI_Button* b) {
    if (!b->editable_content) {
        dst->text = NULL;
        dst->cursor_pos = dst->selection_start = dst->selection_end = 0;
        return;
    }
    size_t len = strlen(b->editable_content) + 1;
    dst->text = (char*)malloc(len);
    if (dst->text) memcpy(dst->text, b->editable_content, len);
    dst->cursor_pos     = b->cursor_pos;
    dst->selection_start = b->selection_start;
    dst->selection_end   = b->selection_end;
}

void push_undo_state(UI_Button* b) {
    if (!b->undo_stack) return;

    // clear redo stack on new edit
    while (b->redo_count > 0) {
        free_editor_state(&b->redo_stack[--b->redo_count]);
    }

    // if full, drop oldest undo
    if (b->undo_count >= b->undo_capacity) {
        free_editor_state(&b->undo_stack[0]);
        memmove(b->undo_stack, b->undo_stack + 1,
                (b->undo_count - 1) * sizeof(EditorState));
        b->undo_count--;
    }

    copy_editor_state(&b->undo_stack[b->undo_count], b);
    b->undo_count++;
}

void restore_state(UI_Button* b, EditorState* s) {
    if (!s || !s->text) return;

    size_t len = strlen(s->text) + 1;
    ensure_capacity(b, len);
    memcpy(b->editable_content, s->text, len);

    b->cursor_pos       = s->cursor_pos;
    b->selection_start  = s->selection_start;
    b->selection_end    = s->selection_end;
    b->content_height   = 0.0f;   // invalidate layout
}

void init_editor_undo(UI_Button* b, int max_steps) {
    b->max_undo_steps = max_steps > 0 ? max_steps : 200;
    b->undo_stack = (EditorState*)malloc(b->max_undo_steps * sizeof(EditorState));
    b->redo_stack = (EditorState*)malloc(b->max_undo_steps * sizeof(EditorState));
    b->undo_count = b->redo_count = 0;
    b->undo_capacity = b->redo_capacity = b->max_undo_steps;

    // push initial state
    push_undo_state(b);
}

void perform_undo(UI_Button* b) {
    if (b->undo_count <= 1) return;   // at least keep the initial state

    // push current state to redo
    if (b->redo_count >= b->redo_capacity) {
        free_editor_state(&b->redo_stack[0]);
        memmove(b->redo_stack, b->redo_stack + 1,
                (b->redo_count - 1) * sizeof(EditorState));
        b->redo_count--;
    }
    copy_editor_state(&b->redo_stack[b->redo_count], b);
    b->redo_count++;

    // restore previous state
    b->undo_count--;
    restore_state(b, &b->undo_stack[b->undo_count - 1]);
}

void perform_redo(UI_Button* b) {
    if (b->redo_count == 0) return;

    // push current to undo
    if (b->undo_count >= b->undo_capacity) {
        free_editor_state(&b->undo_stack[0]);
        memmove(b->undo_stack, b->undo_stack + 1,
                (b->undo_count - 1) * sizeof(EditorState));
        b->undo_count--;
    }
    copy_editor_state(&b->undo_stack[b->undo_count], b);
    b->undo_count++;

    // restore from redo
    b->redo_count--;
    restore_state(b, &b->redo_stack[b->redo_count]);
}
