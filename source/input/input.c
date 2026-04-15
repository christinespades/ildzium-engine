#include "pch.h"
#include "input.h"

void init_input()
{
    set_mouse_callbacks();
    set_key_callbacks();
    #ifndef __EMSCRIPTEN__
        glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    #else
        emscripten_request_pointerlock("#canvas", 1);
    #endif
}

#ifndef __EMSCRIPTEN__
    void input_toggle_mode(GLFWwindow* window)
    {
        g_ui_ctx->cursor_captured = !g_ui_ctx->cursor_captured;

        glfwSetInputMode(window,
            GLFW_CURSOR,
            g_ui_ctx->cursor_captured ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
#else
    void input_toggle_mode()
    {
        g_ui_ctx->cursor_captured = !g_ui_ctx->cursor_captured;

        if (!g_ui_ctx->cursor_captured) {
            emscripten_request_pointerlock("#canvas", 1);
        } else {
            emscripten_exit_pointerlock();
        }
        printf("Toggled input, showing UI: %d\n", g_ui_ctx->cursor_captured);
    }
#endif