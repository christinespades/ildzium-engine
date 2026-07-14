#include "pch.h"
#ifndef __EMSCRIPTEN__
    #include "input/keys/input_keys_vk.h"

    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
    }

    void character_callback(GLFWwindow* window, unsigned int codepoint)
    {
        if (!g_ui_ctx->cursor_captured) return;

        if (g_ui_ctx->active_button_index <= 0) {
            // Find focused editable button
            UI_Button* b = &g_ui_ctx->buttons[g_ui_ctx->active_button_index];
            if (b->is_editable) {
                // Insert character at cursor
                insert_char_at_cursor(b, (char)codepoint);
                return;
            }
        }
    }

    void set_key_callbacks() {
        glfwSetKeyCallback(g_window, key_callback);
        glfwSetCharCallback(g_window, character_callback);
    }
#endif