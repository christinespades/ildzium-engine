#pragma once

#include <stdint.h>

typedef struct UI_Button {
    int x, y;           // top-left position
    int w, h;           // width and height
    const char* text;   // text displayed on button
    void (*on_click)(void); // callback when clicked
} UI_Button;

typedef struct UI_Context {
    UI_Button* buttons;
    int button_count;
    int cursor_captured; // 1 if cursor visible, 0 if captured
} UI_Context;

UI_Context ui_ctx;

void ui_init(UI_Context* ctx);

typedef void (*UI_ButtonCallback)(void);
void ui_add_button(UI_Context* ctx, int x, int y, int w, int h, 
                   const char* text, UI_ButtonCallback callback);
void ui_update(UI_Context* ctx, int mouse_x, int mouse_y, int mouse_pressed);
void ui_draw(UI_Context* ctx, uint32_t* framebuffer, int fb_width, int fb_height);