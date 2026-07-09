#pragma once
#include "ui/ui_button.h"
#include "ui/ui_mode.h"

typedef struct UI_Context {
    UI_Button* buttons;
    int button_count;
    int cursor_captured;
    uint8_t* button_held_last_frame;
    UI_Mode current_mode;

    // Persistent Tooltip Tracking State ===
    const char* active_tooltip_text;
    int   hovered_button_x, hovered_button_y;
    int   hovered_button_w, hovered_button_h;
    int   popup_x, popup_y, popup_w, popup_h;
    float tooltip_scroll_x, tooltip_scroll_y;
} UI_Context;

extern UI_Context* g_ui_ctx;   // global so callbacks can reach it
