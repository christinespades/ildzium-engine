#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "ui/ui_editor_state.h"

typedef struct UI_Button {
    int x, y;           // top-left position
    int w, h;           // width and height
    const char* text;   // text displayed on button
    void (*on_click)(void);
    void (*on_held)(void);
    void (*on_release)(void);
    void (*on_hover)(void);

    float* target_value;   // NULL = normal button, otherwise we increment this
    float  step_size;      // how much to add while held
    float  min_value;
    float  max_value;
    float hold_time; 
    bool is_scrollable;
    bool is_editable;           // new
    const char* content;   // read-only (buttons, labels)
    char* editable_content; // mutable (text editor)
    size_t content_capacity;
    int cursor_pos;             // character index
    int selection_start;        // -1 = no selection
    int selection_end;
    float scroll_offset;        // vertical scroll (pixels)
    float scroll_offset_x;      // horizontal scroll (pixels)
    float content_height;       // total height needed for text
    float content_width;        // total width needed for text (longest line)
    int line_height;            // approx pixels per line (set during draw)

    int last_click_pos;
    int click_count;
    double last_click_time;   // use glfwGetTime() or your timer
    bool is_dragging;
    int drag_start_pos;
    int last_click_x, last_click_y;
    char* filepath;

    EditorState* undo_stack;
    int          undo_count;
    int          undo_capacity;

    EditorState* redo_stack;
    int          redo_count;
    int          redo_capacity;

    // optional: limit memory usage
    int          max_undo_steps;   // e.g. 100 or 500
    bool is_color;
    const char* tooltip;
} UI_Button;
