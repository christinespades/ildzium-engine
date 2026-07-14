#pragma once
#include "ui/ui_callbacks.h"
#include "ui/ui_editor_state.h"
#include "core/params/params.h"
#include "core/params/param_type.h"
#include "core/enums.h"

typedef struct UI_Button {
    int x, y;           // top-left position
    int w, h;           // width and height
    const char* text;   // text displayed on button
    void (*on_click)(void);
    void (*on_held)(void);
    void (*on_release)(void);
    void (*on_hover)(void);

    void* target_value;
    ParamType type;
    const EnumDefinition* enum_definition;
    float  step_size;      // how much to add while held
    float  min_value;
    float  max_value;
    float hold_time; 
    bool is_scrollable;
    bool is_editable;           // new
    const char* content;   // read-only (buttons, labels)
    char text_storage[128];    // dedicated local memory backup
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
    const char* tooltip;
    bool is_container;
    int last_direction; // (-1 = decrease, 1 = increase, 0 = uninitialized)
    bool is_typing;
} UI_Button;

typedef struct UI_Context UI_Context; // avoid circular dependency
typedef void (*UI_ButtonCallback)(void);
void ui_add_button(UI_Context* ctx, int x, int y, int w, int h, const char* text,
                   UI_ButtonCallback on_click,
                   UI_ButtonCallback on_held,
                   UI_ButtonCallback on_release,
                   UI_ButtonCallback on_hover,
                   const char* tooltip_text);

void add_top_button(UI_Context *ctx,
                           int *x,
                           int top_y,
                           int btn_w,
                           int spacing,
                           const char *label,
                           UI_ButtonCallback on_click,
                           const char* tooltip);
void add_top_navigation_buttons(UI_Context* ctx);
bool ui_button_has_focus(UI_Button* b);