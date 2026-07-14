#include "pch.h"
#include "ui_draw_tooltip.h"

void calculate_tooltip_metrics(UI_Context* ctx, UI_Button* temp_b, int line_h, int scale) {
    int num_lines = 1;
    int max_line_chars = 0;
    int current_line_chars = 0;

    // CRASH DEFENSE: Ensure the pointer isn't garbage or a reinterpreted float
    if ((uintptr_t)ctx->active_tooltip_text < 0x10000 || (uintptr_t)ctx->active_tooltip_text == 0x3F800000) {
        printf("CRITICAL: active_tooltip_text corrupted with bad address: %p\n", ctx->active_tooltip_text);
        ctx->active_tooltip_text = NULL;
        return;
    }

    for (const char* p = ctx->active_tooltip_text; p && *p; ++p) {
        if (*p == '\n') {
            num_lines++;
            if (current_line_chars > max_line_chars) max_line_chars = current_line_chars;
            current_line_chars = 0;
        } else {
            current_line_chars++;
        }
    }
    if (current_line_chars > max_line_chars) max_line_chars = current_line_chars;
    
    temp_b->content_height = (float)num_lines * line_h;
    float approx_char_w = get_param_float(PARAM_UI_FONT_HEIGHT) * 0.6f * scale; 
    temp_b->content_width = (float)max_line_chars * approx_char_w;
}

// --- HELPER 2: RENDER STANDALONE POPUP TOOLTIPS ---
void draw_ui_tooltip(UI_Context* ctx, uint32_t* fb, int fb_width, int fb_height) {
    if (!ctx->active_tooltip_text) return;

    draw_rect(fb, fb_width, fb_height, ctx->popup_x, ctx->popup_y, ctx->popup_w, ctx->popup_h, get_param_color(PARAM_UI_COLOR_TUNER_PANEL_IDLE));
    
    UI_Button temp_tooltip_b = {
        .x = ctx->popup_x, .y = ctx->popup_y, 
        .w = ctx->popup_w, .h = ctx->popup_h,
        .content = ctx->active_tooltip_text,
        .scroll_offset = ctx->tooltip_scroll_y,
        .scroll_offset_x = ctx->tooltip_scroll_x
    };

    draw_borders(fb, fb_width, fb_height, &temp_tooltip_b);
    
    int padding = 6;
    float visible_h = (float)(temp_tooltip_b.h - 2 * padding);
    float visible_w = (float)(temp_tooltip_b.w - 2 * padding);
    int txt_pad = 6;
    int scale = calculate_text_scale(temp_tooltip_b.content, temp_tooltip_b.w - (txt_pad * 2), temp_tooltip_b.h - (txt_pad * 2), 3, temp_tooltip_b.is_scrollable);
    int line_h = get_param_float(PARAM_UI_FONT_HEIGHT) * scale + 8;

    calculate_tooltip_metrics(ctx, &temp_tooltip_b, line_h, scale);
    if (!ctx->active_tooltip_text) return; // Bailed if corruption was detected

    // Clamp Scroll values safely
    float max_scroll_y = fmaxf(0.0f, temp_tooltip_b.content_height - visible_h);
    temp_tooltip_b.scroll_offset = fmaxf(0.0f, fminf(temp_tooltip_b.scroll_offset, max_scroll_y));
    float max_scroll_x = fmaxf(0.0f, temp_tooltip_b.content_width - visible_w);
    temp_tooltip_b.scroll_offset_x = fmaxf(0.0f, fminf(temp_tooltip_b.scroll_offset_x, max_scroll_x));

    // Render Text Layer
    draw_multiline_text(fb, fb_width, fb_height, 
                        temp_tooltip_b.x + padding - (int)temp_tooltip_b.scroll_offset_x, 
                        temp_tooltip_b.y + padding - (int)temp_tooltip_b.scroll_offset, 
                        ctx->active_tooltip_text, get_param_color(PARAM_UI_COLOR_MAIN_TEXT), scale, line_h, &temp_tooltip_b);

    // Draw Scrollbars if needed
    if (temp_tooltip_b.content_height > visible_h) {
        int sb_w = 6, sb_x = temp_tooltip_b.x + temp_tooltip_b.w - sb_w - 2, sb_y = temp_tooltip_b.y + 2, sb_h = temp_tooltip_b.h - 4;
        draw_rect(fb, fb_width, fb_height, sb_x, sb_y, sb_w, sb_h, 0x33FFFFFF);
        int handle_h = fmaxf(12, (int)(sb_h * (visible_h / temp_tooltip_b.content_height)));
        int handle_y = sb_y + (int)((sb_h - handle_h) * (temp_tooltip_b.scroll_offset / max_scroll_y));
        draw_rect(fb, fb_width, fb_height, sb_x, handle_y, sb_w, handle_h, get_param_color(PARAM_UI_COLOR_MAIN_TEXT));
    }
    if (temp_tooltip_b.content_width > visible_w) {
        int sb_h = 6, sb_x = temp_tooltip_b.x + 2, sb_y = temp_tooltip_b.y + temp_tooltip_b.h - sb_h - 2, sb_w = temp_tooltip_b.w - 4;
        draw_rect(fb, fb_width, fb_height, sb_x, sb_y, sb_w, sb_h, 0x33FFFFFF);
        int handle_w = fmaxf(12, (int)(sb_w * (visible_w / temp_tooltip_b.content_width)));
        int handle_x = sb_x + (int)((sb_w - handle_w) * (temp_tooltip_b.scroll_offset_x / max_scroll_x));
        draw_rect(fb, fb_width, fb_height, handle_x, sb_y, handle_w, sb_h, get_param_color(PARAM_UI_COLOR_MAIN_TEXT));
    }

    ctx->tooltip_scroll_y = temp_tooltip_b.scroll_offset;
    ctx->tooltip_scroll_x = temp_tooltip_b.scroll_offset_x;
}
