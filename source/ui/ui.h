#pragma once
#include <stdint.h>

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
} UI_Button;

typedef enum {
    UI_MODE_MAIN_MENU = 0,
    UI_MODE_SKYBOX,
    UI_MODE_TERRAIN,
    // add more later: UI_MODE_LIGHTS, UI_MODE_CAMERA, etc.
    UI_MODE_COUNT
} UI_Mode;

typedef struct UI_Context {
    UI_Button* buttons;
    int button_count;
    int cursor_captured;
    uint8_t* button_held_last_frame;
    UI_Mode current_mode;   // <--- add this
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
void ui_update(UI_Context* ctx, int mouse_x, int mouse_y, int mouse_pressed, float dt);
void ui_draw(UI_Context* ctx, uint32_t* framebuffer, int fb_width, int fb_height);
void ui_set_mode(UI_Context* ctx, UI_Mode mode);