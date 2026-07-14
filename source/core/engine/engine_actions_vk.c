#include "pch.h"
#ifndef __EMSCRIPTEN__
	#include "engine_actions_vk.h"
    #include "core/window/window_vk.h"

	void engine_exit(const InputEvent *event) {
    	glfwSetWindowShouldClose(g_window, GLFW_TRUE);
    }

	void engine_toggle_mode(const InputEvent *event) {
        g_ui_ctx->cursor_captured = !g_ui_ctx->cursor_captured;

        glfwSetInputMode(g_window,
            GLFW_CURSOR, g_ui_ctx->cursor_captured ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
#endif