#pragma once
#include "ui/ui_button.h"
#include "ui/ui_mode.h"

typedef struct UI_Context {
    UI_Button* buttons;
    int button_count;
    int cursor_captured;
    uint8_t* button_held_last_frame;
    UI_Mode current_mode;
} UI_Context;

extern UI_Context* g_ui_ctx;   // global so callbacks can reach it
