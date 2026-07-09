#include "pch.h"
#include "ui/ui_draw.h"

void draw_editor(UI_Button* b, uint32_t* fb, int fb_width, int fb_height, float dt)
{
    if (!b->is_editable || !b->editable_content || b->editable_content[0] == '\0')
        return;

    // Safety defaults
    if (b->line_height <= 0)
        b->line_height = get_param_float(PARAM_UI_FONT_HEIGHT) + 8;   // typical value

    const char* text = b->editable_content;
    int padding = 12;
    int line_numbers_w = get_param_float(PARAM_UI_EDITOR_LINE_NUMBERS_WIDTH);
    int text_x_base = b->x + padding + line_numbers_w;
    int text_y_base = b->y + padding;

    // === Recalculate content height & width only when needed ===
    if (b->content_height <= 0.0f || b->content_width <= 0.0f)
    {
        int num_lines = 1;
        int current_line_chars = 0;
        int max_line_chars = 0;

        for (const char* p = text; *p; ++p)
        {
            if (*p == '\n')
            {
                num_lines++;
                if (current_line_chars > max_line_chars) max_line_chars = current_line_chars;
                current_line_chars = 0;
            }
            else
            {
                current_line_chars++;
            }
        }
        if (current_line_chars > max_line_chars) max_line_chars = current_line_chars;

        b->content_height = (float)num_lines * b->line_height;
        b->content_width = (float)max_line_chars * get_param_float(PARAM_UI_FONT_WIDTH);
    }

    // Clamp cursor
    size_t text_len = strlen(text);
    if (b->cursor_pos < 0)          b->cursor_pos = 0;
    if ((size_t)b->cursor_pos > text_len) b->cursor_pos = (int)text_len;

    // === Clamp vertical and horizontal scroll offsets ===
    float visible_h = (float)(b->h - 2 * padding);
    float max_scroll_y = fmaxf(0.0f, b->content_height - visible_h);
    b->scroll_offset = fmaxf(0.0f, fminf(b->scroll_offset, max_scroll_y));

    // Visible width takes the line numbers column into account
    float visible_w = (float)(b->w - 2 * padding - line_numbers_w);
    float max_scroll_x = fmaxf(0.0f, b->content_width - visible_w);
    b->scroll_offset_x = fmaxf(0.0f, fminf(b->scroll_offset_x, max_scroll_x));

    // Final shifted drawing coordinates
    int draw_x = text_x_base - (int)b->scroll_offset_x;
    int draw_y = text_y_base - (int)b->scroll_offset;

    // 1. Line Numbers (Stays statically left; only scrolls vertically)
    draw_line_numbers(b, fb, fb_width, fb_height, b->x + padding, draw_y);

    // 2. Selection Highlighting (Must use shifted draw_x)
    if (b->selection_start != -1)
        draw_selection_highlighting(b, fb, fb_width, fb_height, draw_x, draw_y);

    // 3. Main text (Must use shifted draw_x)
    draw_multiline_text(fb, fb_width, fb_height,
                        draw_x, draw_y,
                        text, get_param_color(PARAM_UI_COLOR_EDITOR_MAIN_TEXT), 1, b->line_height, b);

    // 4. Blinking Caret (Must apply horizontal scroll)
    static float blink_timer = 0.0f;
    blink_timer += dt;
    if (((int)(blink_timer * 3.0f) % 2) == 0)
    {
        int cursor_line = 0;
        int cursor_col = 0;
        int idx = 0;

        for (const char* p = text; *p && idx < b->cursor_pos; ++p, ++idx)
        {
            if (*p == '\n')
            {
                cursor_line++;
                cursor_col = 0;
            }
            else
            {
                cursor_col++;
            }
        }

        // Apply horizontal scroll to screen position
        int cursor_screen_x = draw_x + cursor_col * get_param_float(PARAM_UI_FONT_WIDTH);
        int cursor_screen_y = draw_y + cursor_line * b->line_height;

        // Clip caret to the text bounds area (so it doesn't leak into line numbers or margins)
        if (cursor_screen_y + b->line_height > b->y && cursor_screen_y < b->y + b->h &&
            cursor_screen_x >= text_x_base && cursor_screen_x < b->x + b->w - padding)
        {
            draw_rect(fb, fb_width, fb_height,
                      cursor_screen_x, cursor_screen_y,
                      2, b->line_height,
                      get_param_color(PARAM_UI_COLOR_EDITOR_CARET));
        }
    }
}

static int calculate_text_scale(const char* text, int max_w, int max_h, int max_target_scale, bool is_scrollable)
{
    if (!text || text[0] == '\0' || max_w <= 0 || max_h <= 0) 
        return 1;

    // --- Step 1: Analyze text contents ---
    int max_chars_in_line = 0;
    int current_chars = 0;
    int num_lines = 1;

    for (const char* p = text; *p; ++p) {
        if (*p == '\n') {
            if (current_chars > max_chars_in_line) {
                max_chars_in_line = current_chars;
            }
            current_chars = 0;
            num_lines++;
        } else {
            current_chars++;
        }
    }
    if (current_chars > max_chars_in_line) {
        max_chars_in_line = current_chars;
    }

    // --- Step 2: Downscale loop ---
    // Start at our desired huge size and test backwards until it fits
    for (int scale = max_target_scale; scale > 1; scale--) {
        int text_w = max_chars_in_line * (get_param_float(PARAM_UI_FONT_WIDTH) * scale);
        
        // If it's scrollable, we only care if it fits horizontally
        if (is_scrollable) {
            if (text_w <= max_w) return scale;
        } else {
            // Non-scrollable: must fit both width and height boundaries
            int line_h = get_param_float(PARAM_UI_FONT_HEIGHT) * scale + 8;
            int text_h = num_lines * line_h;
            if (text_w <= max_w && text_h <= max_h) return scale;
        }
    }

    return 1; // Fallback to minimum usable scale
}

void ui_draw(UI_Context* ctx, uint32_t* fb, int fb_width, int fb_height, float dt)
{
    if (!ctx || !fb || fb_width <= 0 || fb_height <= 0 || !ctx->cursor_captured)
        return;
    
    extern double g_mouse_x, g_mouse_y;

    // --- 1. STATE CLEANUP AT THE START OF THE FRAME ---
    // Instead of sticky tracking, assume no hover, let the loop find if we are hovering.
    const char* found_tooltip = NULL;
    UI_Button* hovered_b = NULL;

    // Check if we are hovering over the existing popup window space from last frame
    bool hovering_popup = false;
    if (ctx->active_tooltip_text) {
        hovering_popup = (g_mouse_x >= ctx->popup_x && g_mouse_x < ctx->popup_x + ctx->popup_w &&
                          g_mouse_y >= ctx->popup_y && g_mouse_y < ctx->popup_y + ctx->popup_h);
    }

    memset(fb, 0, (size_t)fb_width * fb_height * sizeof(uint32_t));
    for (int i = 0; i < ctx->button_count; i++) {
        UI_Button* b = &ctx->buttons[i];

        bool is_hover = (g_mouse_x >= b->x && g_mouse_x < b->x + b->w && g_mouse_y >= b->y && g_mouse_y < b->y + b->h); 
        if (is_hover && b->tooltip) {
            found_tooltip = b->tooltip;
            hovered_b = b;
        }

        // Layout container box
        if (b->on_click == on_layout_box_clicked) {
            // Draw Container Panel Base Background Layer
            draw_rect(fb, fb_width, fb_height, b->x, b->y, b->w, b->h, 
                      get_param_color(PARAM_UI_COLOR_TUNER_PANEL_IDLE));
            
            // Draw outer border outline around housing frame box
            draw_borders(fb, fb_width, fb_height, b);

            int padding = 12;
            float visible_h = (float)(b->h - 2 * padding);
            float visible_w = (float)(b->w - 2 * padding);

            // Handle Vertical Scrollbar Overlay for Macro Panel Window
            if (b->content_height > visible_h) {
                draw_scrollbar(b, fb, fb_width, fb_height);
            }

            // Handle Horizontal Scrollbar Overlay for Macro Panel Window
            if (b->content_width > visible_w) {
                draw_horizontal_scrollbar(b, fb, fb_width, fb_height);
            }
            continue; // Advance loop past normal child button evaluation handlers
        }

        // Background
        uint32_t panel_color = (b->target_value && ctx->button_held_last_frame[i])
                               ? get_param_color(PARAM_UI_COLOR_TUNER_PANEL_ACTIVE) : get_param_color(PARAM_UI_COLOR_TUNER_PANEL_IDLE);
        draw_rect(fb, fb_width, fb_height, b->x, b->y, b->w, b->h, panel_color);

        draw_borders(fb, fb_width, fb_height, b);

        // Tuner split line
        if (b->target_value) {
            int mid = b->x + b->w / 2;
            draw_rect(fb, fb_width, fb_height, mid, b->y + 4, 2, b->h - 8, get_param_color(PARAM_UI_COLOR_TUNER_SPLIT_LINE));

            // Current value
            char val[32];
            if (b->is_color) {
                // Reinterpret the target value back as a crisp 32-bit uint color hex string
                uint32_t color_val = *(uint32_t*)b->target_value;
                snprintf(val, sizeof(val), "0x%08X", color_val);
            } else {
                // Standard float rendering
                float value = *b->target_value; 
                snprintf(val, sizeof(val), "%.3f", value);
            }
            draw_text(fb, fb_width, fb_height, val, b->x + 20, b->y + b->h - 16, get_param_color(PARAM_UI_COLOR_TUNER_VALUE_TEXT), 2, b);
        }

        // Draw editor if it's an editable text area
        if (b->is_editable && b->editable_content) {
            draw_editor(b, fb, fb_width, fb_height, dt);

            // Dynamic visibility checks based on available space inside the padding (12px)
            int padding = 12; 
            float visible_h = (float)(b->h - 2 * padding);
            float visible_w = (float)(b->w - 2 * padding);

            // Only render the vertical scrollbar if editor content overflows vertically
            if (b->content_height > visible_h) {
                draw_scrollbar(b, fb, fb_width, fb_height);
            }

            // Only render the horizontal scrollbar if editor content overflows horizontally
            if (b->content_width > visible_w) {
                draw_horizontal_scrollbar(b, fb, fb_width, fb_height);
            }
        }
        
        // Normal scrollable or static text button
        else if (b->content && b->content[0])
        {
            int padding = 6;
            // Calculate actual available workspace inside the button boundaries
            int max_avail_w = b->w - (padding * 2);
            int max_avail_h = b->h - (padding * 2);

            // Dynamically calculate the safe layout scale (up to 3)
            int scale = calculate_text_scale(b->content, max_avail_w, max_avail_h, 3, b->is_scrollable);
            int line_h = get_param_float(PARAM_UI_FONT_HEIGHT) * scale + 8;

            if (b->is_scrollable)
            {
                // === Reset / Calculate content bounds if dirty ===
                if (b->content_height <= 0.0f)
                {
                    int num_lines = 1;
                    for (const char* p = b->content; *p; ++p)
                        if (*p == '\n') num_lines++;
                    b->content_height = (float)num_lines * line_h;
                }

                int tx = b->x + padding;
                int ty = b->y + padding;

                float visible_h = (float)(b->h - 2 * padding);
                float visible_w = (float)(b->w - 2 * padding);

                // Clamp Vertical Scroll
                float max_scroll_y = fmaxf(0.0f, b->content_height - visible_h);
                b->scroll_offset = fmaxf(0.0f, fminf(b->scroll_offset, max_scroll_y));

                // Clamp Horizontal Scroll
                float max_scroll_x = fmaxf(0.0f, b->content_width - visible_w);
                b->scroll_offset_x = fmaxf(0.0f, fminf(b->scroll_offset_x, max_scroll_x));

                // Notice we pass: tx - scroll_offset_x
                draw_multiline_text(fb, fb_width, fb_height,
                                    tx - (int)b->scroll_offset_x, ty - (int)b->scroll_offset,
                                    b->content, get_param_color(PARAM_UI_COLOR_MAIN_TEXT), scale, line_h, b);

                // Render Vertical Scrollbar if needed
                if (b->content_height > visible_h) {
                    draw_scrollbar(b, fb, fb_width, fb_height);
                }

                // Render Horizontal Scrollbar if needed
                if (b->content_width > visible_w) {
                    draw_horizontal_scrollbar(b, fb, fb_width, fb_height);
                }
            }
            else
            {
                // non-scrollable
                draw_multiline_text(fb, fb_width, fb_height,
                                    b->x + padding, b->y + padding,
                                    b->content, get_param_color(PARAM_UI_COLOR_MAIN_TEXT), scale, line_h, b);
            }
        }
    }

    // --- 3. EVALUATE TOOLTIP LIFETIME ---
    if (found_tooltip && hovered_b) {
        ctx->active_tooltip_text = found_tooltip;
        ctx->popup_w = get_param_float(PARAM_UI_POPUP_WIDTH);
        ctx->popup_h = get_param_float(PARAM_UI_POPUP_HEIGHT);
        ctx->popup_x = hovered_b->x;
        ctx->popup_y = hovered_b->y + hovered_b->h + 2;
    } 
    else if (!hovering_popup) {
        ctx->active_tooltip_text = NULL;
    }

    // --- 4. STANDALONE TOOLTIP RENDERING ---
    if (ctx->active_tooltip_text) {
        // Draw tooltip background base container window
        draw_rect(fb, fb_width, fb_height, ctx->popup_x, ctx->popup_y, ctx->popup_w, ctx->popup_h, get_param_color(PARAM_UI_COLOR_TUNER_PANEL_IDLE));
        
        // Setup a local stack structure that stays safe at its own scope address
        UI_Button temp_tooltip_b = {0};
        temp_tooltip_b.x = ctx->popup_x;
        temp_tooltip_b.y = ctx->popup_y;
        temp_tooltip_b.w = ctx->popup_w;
        temp_tooltip_b.h = ctx->popup_h;
        temp_tooltip_b.content = ctx->active_tooltip_text;
        temp_tooltip_b.scroll_offset = ctx->tooltip_scroll_y;
        temp_tooltip_b.scroll_offset_x = ctx->tooltip_scroll_x;

        draw_borders(fb, fb_width, fb_height, &temp_tooltip_b);
        
        int padding = 6;
        float visible_h = (float)(temp_tooltip_b.h - 2 * padding);
        float visible_w = (float)(temp_tooltip_b.w - 2 * padding);
        int scale = 1;
        int line_h = get_param_float(PARAM_UI_FONT_HEIGHT) * scale + 6;

        // 1. Calculate Heights and Width Metrics manually and safely
        int num_lines = 1;
        int max_line_chars = 0;
        int current_line_chars = 0;
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
        
        temp_tooltip_b.content_height = (float)num_lines * line_h;

        float approx_char_w = get_param_float(PARAM_UI_FONT_HEIGHT) * 0.6f * scale; 
        temp_tooltip_b.content_width = (float)max_line_chars * approx_char_w;

        // Clamp Scroll values based on runtime properties
        float max_scroll_y = fmaxf(0.0f, temp_tooltip_b.content_height - visible_h);
        temp_tooltip_b.scroll_offset = fmaxf(0.0f, fminf(temp_tooltip_b.scroll_offset, max_scroll_y));

        float max_scroll_x = fmaxf(0.0f, temp_tooltip_b.content_width - visible_w);
        temp_tooltip_b.scroll_offset_x = fmaxf(0.0f, fminf(temp_tooltip_b.scroll_offset_x, max_scroll_x));

        // 2. Draw typography string layer with manual pixel offsets
        draw_multiline_text(fb, fb_width, fb_height, 
                            temp_tooltip_b.x + padding - (int)temp_tooltip_b.scroll_offset_x, 
                            temp_tooltip_b.y + padding - (int)temp_tooltip_b.scroll_offset, 
                            ctx->active_tooltip_text, get_param_color(PARAM_UI_COLOR_MAIN_TEXT), scale, line_h, &temp_tooltip_b);

        // 3. DRAW CUSTOM TOOLTIP SCROLLBARS (Bypasses ID lookup checking)
        // Draw Vertical Track & Thumb if text overflows height
        if (temp_tooltip_b.content_height > visible_h) {
            int sb_w = 6;
            int sb_x = temp_tooltip_b.x + temp_tooltip_b.w - sb_w - 2;
            int sb_y = temp_tooltip_b.y + 2;
            int sb_h = temp_tooltip_b.h - 4;
            
            // Draw background bar track line
            draw_rect(fb, fb_width, fb_height, sb_x, sb_y, sb_w, sb_h, 0x33FFFFFF);
            
            // Calculate active proportional handle thumb sizes
            float handle_ratio = visible_h / temp_tooltip_b.content_height;
            int handle_h = (int)(sb_h * handle_ratio);
            if (handle_h < 12) handle_h = 12; // safety constraint
            
            float scroll_ratio = temp_tooltip_b.scroll_offset / max_scroll_y;
            int handle_y = sb_y + (int)((sb_h - handle_h) * scroll_ratio);
            
            // Draw moving drag thumb knob block
            draw_rect(fb, fb_width, fb_height, sb_x, handle_y, sb_w, handle_h, get_param_color(PARAM_UI_COLOR_MAIN_TEXT));
        }

        // Draw Horizontal Track & Thumb if text overflows width
        if (temp_tooltip_b.content_width > visible_w) {
            int sb_h = 6;
            int sb_x = temp_tooltip_b.x + 2;
            int sb_y = temp_tooltip_b.y + temp_tooltip_b.h - sb_h - 2;
            int sb_w = temp_tooltip_b.w - 4;
            
            draw_rect(fb, fb_width, fb_height, sb_x, sb_y, sb_w, sb_h, 0x33FFFFFF);
            
            float handle_ratio = visible_w / temp_tooltip_b.content_width;
            int handle_w = (int)(sb_w * handle_ratio);
            if (handle_w < 12) handle_w = 12;
            
            float scroll_ratio = temp_tooltip_b.scroll_offset_x / max_scroll_x;
            int handle_x = sb_x + (int)((sb_w - handle_w) * scroll_ratio);
            
            draw_rect(fb, fb_width, fb_height, handle_x, sb_y, handle_w, sb_h, get_param_color(PARAM_UI_COLOR_MAIN_TEXT));
        }

        // Write updates back into parent context trackers safely
        ctx->tooltip_scroll_y = temp_tooltip_b.scroll_offset;
        ctx->tooltip_scroll_x = temp_tooltip_b.scroll_offset_x;
    } else {
        ctx->tooltip_scroll_y = 0.0f;
        ctx->tooltip_scroll_x = 0.0f;
    }

    ui_draw_info(fb, fb_width, fb_height, dt); // fps, ui focus, debug, etc
}

// ====================== SELECTION & LINE NUMBERS ======================
// Draw accurate per-line selection highlighting
// For both read-only text (content) and mutable text (editable_content)
// Draw accurate per-character selection highlighting where the selected characters are (not at line start) and highlights the selected range (word, multiple words, lines, etc.)
void draw_selection_highlighting(UI_Button* b, uint32_t* fb, int fb_width, int fb_height, int text_x, int draw_y)
{
    const char* text = b->is_editable ? b->editable_content : b->content;
    if (b->selection_start == -1 || !text) return;

    int sel_start = b->selection_start < b->selection_end ? b->selection_start : b->selection_end;
    int sel_end   = b->selection_start > b->selection_end ? b->selection_start : b->selection_end;
    if (sel_start == sel_end) return;

    int line_height = b->line_height > 0 ? b->line_height : (get_param_float(PARAM_UI_FONT_HEIGHT) + 4);
    float font_w = get_param_float(PARAM_UI_FONT_WIDTH);

    int current_line = 0;
    int char_idx = 0;
    const char* p = text;

    while (*p && char_idx < sel_end)
    {
        int line_start_idx = char_idx;
        int line_sel_start = -1;
        int line_sel_end = -1;

        // Process the current line to find the batch selection range
        while (*p && *p != '\n')
        {
            if (char_idx >= sel_start && char_idx < sel_end)
            {
                if (line_sel_start == -1) {
                    line_sel_start = char_idx - line_start_idx;
                }
                line_sel_end = char_idx - line_start_idx + 1;
            }
            p++;
            char_idx++;
        }

        // Draw ONE rectangle for this entire line's selection (if it falls in range)
        if (line_sel_start != -1)
        {
            int highlight_x = text_x + (int)(line_sel_start * font_w);
            int highlight_w = (int)((line_sel_end - line_sel_start) * font_w);
            int highlight_y = draw_y + current_line * line_height;

            draw_rect(fb, fb_width, fb_height,
                      highlight_x, highlight_y,
                      highlight_w, line_height,
                      get_param_color(PARAM_UI_COLOR_TEXT_HIGHLIGHT));
        }

        // Handle newline transition
        if (*p == '\n')
        {
            p++;
            char_idx++;
            current_line++;
        }
    }
}

// Draw line numbers
void draw_line_numbers(UI_Button* b, uint32_t* fb, int fb_width, int fb_height,
                       int text_x, int draw_y)
{
    if (!b->editable_content) return;

    int line_height = b->line_height;
    int num_lines = 1;
    for (const char* p = b->editable_content; *p; p++) {
        if (*p == '\n') num_lines++;
    }

    int line_num_x = b->x + 8;
    int current_line = 1;
    char num_buf[16];

    // Calculate the exact top and bottom limits based on your editor padding
    int padding = 12;
    int clip_t = b->y + padding;
    int clip_b = b->y + b->h - padding;

    for (int i = 0; i < num_lines; i++)
    {
        int y_pos = draw_y + i * line_height;

        // Tighter visibility check: Only draw if the line number sits entirely within the text zone
        if (y_pos >= clip_t && (y_pos + get_param_float(PARAM_UI_FONT_HEIGHT)) <= clip_b)
        {
            snprintf(num_buf, sizeof(num_buf), "%3d", current_line);
            draw_text(fb, fb_width, fb_height,
                      num_buf,
                      line_num_x,
                      y_pos,
                      get_param_color(PARAM_UI_COLOR_EDITOR_LINE_NUMBERS), 1, b);
        }
        current_line++;
    }
}