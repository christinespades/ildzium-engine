#pragma once

typedef struct {
    char* text;          // full content at that moment
    int   cursor_pos;
    int   selection_start;
    int   selection_end;
} EditorState;
