#include "pch.h"
#include "input/input_keys.h"

#ifdef __EMSCRIPTEN__
    EM_BOOL key_down(int eventType, const EmscriptenKeyboardEvent* e, void* userData)
    {
        platform_key key = translate_code(e->code);
        printf("KEY DOWN: %s\n", e->code);
        if (key >= 0 && key < 512) {
            g_keys[key] = 1;
            if (g_ui_ctx->cursor_captured) {
	            handle_editor_key(key);
	        }
        }
        return 0;
    }

    EM_BOOL key_up(int eventType, const EmscriptenKeyboardEvent* e, void* userData)
    {
        platform_key key = translate_code(e->code);
        if (key >= 0 && key < 512) {
            g_keys[key] = 0;
        }
        return 0;
    }
#else
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (!g_ui_ctx) return;
        if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

        // Handle universal key actions first
        if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
            input_toggle_mode(window);
        }
        else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        // Route editing keys when UI is captured
        else if (g_ui_ctx->cursor_captured) {
            handle_editor_key((platform_key)key);
        }
    }

    void character_callback(GLFWwindow* window, unsigned int codepoint)
    {
        if (!g_ui_ctx->cursor_captured) return;

        // Find focused editable button (for now assume only one, or add focus logic)
        for (int i = 0; i < g_ui_ctx->button_count; i++) {
            UI_Button* b = &g_ui_ctx->buttons[i];
            if (b->is_editable) {
                // Insert character at cursor
                insert_char_at_cursor(b, (char)codepoint);
                return;
            }
        }
    }
#endif

void set_key_callbacks() {
	#ifdef __EMSCRIPTEN__
        emscripten_set_keydown_callback("#canvas", 0, 1, key_down);
        emscripten_set_keyup_callback("#canvas", 0, 1, key_up);
    #else
        glfwSetKeyCallback(g_window, key_callback);
        glfwSetCharCallback(g_window, character_callback);
    #endif
}