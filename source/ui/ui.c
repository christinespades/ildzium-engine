#include "pch.h"
#include "ui/ui.h"

UI_Context* g_ui_ctx = {0};
static float hold_accumulator = 0.0f;
#ifndef __EMSCRIPTEN__
    extern GLFWwindow* g_window; 
#endif

void ui_init(UI_Context* ctx) {
    ctx->buttons = malloc(sizeof(UI_Button) * get_param_float(PARAM_UI_MAX_BUTTONS));
    ctx->button_held_last_frame = calloc(get_param_float(PARAM_UI_MAX_BUTTONS), sizeof(uint8_t)); // zero-initialized
    ctx->button_count = 0;
    ctx->active_button_index = -1;
#ifdef __EMSCRIPTEN__
    ctx->cursor_captured = 1;
#else
    ctx->cursor_captured = 0;        // start with cursor disabled (camera mode)
#endif
    g_ui_ctx = ctx;
    ctx->current_mode = UI_MODE_MAIN;
    ui_set_mode(ctx, UI_MODE_MAIN);   // builds top bar + main menu
}

void ui_cleanup(UI_Context* ctx) {
    if (ctx->buttons) free(ctx->buttons);
    if (ctx->button_held_last_frame) free(ctx->button_held_last_frame);
    ctx->buttons = NULL;
    ctx->button_held_last_frame = NULL;
    ctx->button_count = 0;
}