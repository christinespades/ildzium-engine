#include "pch.h"
#include "ui/ui_callbacks.h"

void ui_dispatch_callbacks(UI_Button* b, int is_active, int mouse_pressed, int mouse_pressed_this_frame, int mouse_released_this_frame)
{
    if (is_active && mouse_pressed_this_frame) {
        if (b->on_click) ((void (*)(UI_Button*))b->on_click)(b);
        // LOGI("Clicked button: %s", b->content);
    }
    if (is_active && mouse_pressed) {
        if (b->on_held) ((void (*)(UI_Button*))b->on_held)(b);
    }
    if (is_active && mouse_released_this_frame) {
        if (b->on_release) ((void (*)(UI_Button*))b->on_release)(b);
    }
}

// Generic event whenever any button is held down
void on_button_held(void) {
    //printf("Button HELD!\n");
}

void on_button_hovered(void) {
}

void on_layout_box_clicked(void) {}

#define X(name, mode) \
    void on_##name##_clicked(void) { if (g_ui_ctx) ui_set_mode(g_ui_ctx, mode); }
UI_MODES_MAP
#undef X