#include "pch.h"
#ifdef __EMSCRIPTEN__
	#include "engine_actions_webgpu.h"

	void engine_toggle_mode(const InputEvent *event) {
        g_ui_ctx->cursor_captured = !g_ui_ctx->cursor_captured;

        if (!g_ui_ctx->cursor_captured) {
            emscripten_request_pointerlock("#canvas", 1);
        } else {
            emscripten_exit_pointerlock();
        }
        printf("Toggled input, showing UI: %d\n", g_ui_ctx->cursor_captured);
    }
#endif