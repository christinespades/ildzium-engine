#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char* text;          // full content at that moment
    int   cursor_pos;
    int   selection_start;
    int   selection_end;
} EditorState;

typedef struct UI_Button {
    int x, y;           // top-left position
    int w, h;           // width and height
    const char* text;   // text displayed on button
    void (*on_click)(void);
    void (*on_held)(void);
    void (*on_release)(void);

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
    float content_height;       // total height needed for text
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
} UI_Button;

typedef enum {
    UI_MODE_CAMERA = 0,
    UI_MODE_FX,
    UI_MODE_INPUT,
    UI_MODE_LIGHTS,
    UI_MODE_MAIN,
    UI_MODE_MESHES,
    UI_MODE_SKYBOX,
    UI_MODE_SOUNDS,
    UI_MODE_TERRAIN,
    UI_MODE_COUNT
} UI_Mode;

typedef struct UI_Context {
    UI_Button* buttons;
    int button_count;
    int cursor_captured;
    uint8_t* button_held_last_frame;
    UI_Mode current_mode;
} UI_Context;

UI_Context* g_ui_ctx;   // global so callbacks can reach it

void ui_init(UI_Context* ctx);
void ui_cleanup(UI_Context* ctx);   // important for freeing memory

typedef void (*UI_ButtonCallback)(void);
void ui_add_button(UI_Context* ctx, int x, int y, int w, int h,
                   const char* text,
                   void (*on_click)(void),
                   void (*on_held)(void),
                   void (*on_release)(void));
// Generic tuner button (left = decrease, right = increase)
void ui_add_tuner(UI_Context* ctx, float x, float y, float w, float h,
                  const char* text,
                  float* target,
                  float min_val,
                  float max_val);
void ui_update(UI_Context* ctx, int mouse_x, int mouse_y, int mouse_pressed, int mouse_wheel, float dt);
void ui_set_mode(UI_Context* ctx, UI_Mode mode);