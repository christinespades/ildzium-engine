#include "pch.h"
#include "ui/ui_draw.h"

void ui_draw()
{
    if (!g_ui_ctx || !g_ui_fb || g_width <= 0 || g_height <= 0 || !g_ui_ctx->cursor_captured)
        return;
    
    extern double g_mouse_x, g_mouse_y;
    const char* found_tooltip = NULL;
    UI_Button* hovered_b = NULL;

    bool hovering_popup = (g_ui_ctx->active_tooltip_text) && 
                          (g_mouse_x >= g_ui_ctx->popup_x && g_mouse_x < g_ui_ctx->popup_x + g_ui_ctx->popup_w &&
                           g_mouse_y >= g_ui_ctx->popup_y && g_mouse_y < g_ui_ctx->popup_y + g_ui_ctx->popup_h);

    memset(g_ui_fb, 0, (size_t)g_width * g_height * sizeof(uint32_t));

    // Process and draw elements loop
    for (int i = 0; i < g_ui_ctx->button_count; i++) {
        UI_Button* b = &g_ui_ctx->buttons[i];

        // Track tooltips during draw processing pass
        if (g_mouse_x >= b->x && g_mouse_x < b->x + b->w && g_mouse_y >= b->y && g_mouse_y < b->y + b->h) {
            if (b->tooltip && b->tooltip[0]) {
                found_tooltip = b->tooltip;
                hovered_b = b;
            }
        }
        draw_ui_single_button(g_ui_ctx, b, g_ui_fb, g_width, g_height, i, g_dt);
    }

    // Process and evaluate tooltip life bounds
    if (found_tooltip && hovered_b) {
        g_ui_ctx->active_tooltip_text = found_tooltip;
        g_ui_ctx->popup_w = get_param_float(PARAM_UI_POPUP_WIDTH);
        g_ui_ctx->popup_h = get_param_float(PARAM_UI_POPUP_HEIGHT);
        g_ui_ctx->popup_x = hovered_b->x;
        g_ui_ctx->popup_y = hovered_b->y + hovered_b->h + 2;
    } else if (!hovering_popup) {
        g_ui_ctx->active_tooltip_text = NULL;
    }

    // Render standalone tooltip windows safely
    if (g_ui_ctx->active_tooltip_text) {
        draw_ui_tooltip(g_ui_ctx, g_ui_fb, g_width, g_height);
    } else {
        g_ui_ctx->tooltip_scroll_y = 0.0f;
        g_ui_ctx->tooltip_scroll_x = 0.0f;
    }

    ui_draw_info(g_ui_fb, g_width, g_height, g_dt);
}
