#include "pch.h"
#include "input.h"

void init_input()
{
    set_mouse_callbacks();
    set_key_callbacks();
    #ifndef __EMSCRIPTEN__
        glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
#endif