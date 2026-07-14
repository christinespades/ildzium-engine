#include "pch.h"
#ifdef __EMSCRIPTEN__
    #include "input/keys/input_keys_webgpu.h"

    EM_BOOL key_down(int eventType, const EmscriptenKeyboardEvent* e, void* userData)
    {
        platform_key key = translate_code(e->code);

        if (key >= 0 && key < 512) {
            g_keys[key] = 1;

            // Only treat non-repeats as "press"
            if (!e->repeat) {

                // TAB -> toggle mode
                if (strcmp(e->code, "Tab") == 0) {
                    input_toggle_mode();
                    return 1;
                }

                // ESC -> exit / unlock
                if (strcmp(e->code, "Escape") == 0) {
                    emscripten_exit_pointerlock();
                    return 1;
                }
            }

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

    EM_BOOL key_press(int eventType, const EmscriptenKeyboardEvent* e, void* userData)
    {
        if (!g_ui_ctx->cursor_captured) return 0;

        char c = e->key[0]; // basic ASCII

        for (int i = 0; i < g_ui_ctx->button_count; i++) {
            UI_Button* b = &g_ui_ctx->buttons[i];
            if (b->is_editable) {
                insert_char_at_cursor(b, c);
                return 1;
            }
        }

        return 0;
    }

    void set_key_callbacks() {
        emscripten_set_keydown_callback("#canvas", 0, 1, key_down);
        emscripten_set_keyup_callback("#canvas", 0, 1, key_up);
        emscripten_set_keypress_callback("#canvas", 0, 1, key_press);
    }
#endif