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
    init_input_actions();
}