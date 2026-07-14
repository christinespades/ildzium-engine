#include "pch.h"
#include "ui_draw_button.h"

// SINGLE BUTTON DRAWING & SCROLL MANAGEMENT ---
void draw_ui_single_button(UI_Context* ctx, UI_Button* b, uint32_t* fb, int fb_width, int fb_height, int index, float dt) {
    extern double g_mouse_x, g_mouse_y;
    int padding = 12;
    float visible_h = (float)(b->h - 2 * padding);
    float visible_w = (float)(b->w - 2 * padding);

    // Layout containers get fast-tracked
    if (b->on_click == on_layout_box_clicked) {
        draw_rect(fb, fb_width, fb_height, b->x, b->y, b->w, b->h, get_param_color(PARAM_UI_COLOR_TUNER_PANEL_IDLE));
        draw_borders(fb, fb_width, fb_height, b);
        if (b->content_height > visible_h) draw_scrollbar(b, fb, fb_width, fb_height);
        if (b->content_width > visible_w)  draw_horizontal_scrollbar(b, fb, fb_width, fb_height);
        return;
    }

    // Render standard background and tracking states
    uint32_t panel_color = (b->target_value && ctx->button_held_last_frame[index])
                           ? get_param_color(PARAM_UI_COLOR_TUNER_PANEL_ACTIVE) : get_param_color(PARAM_UI_COLOR_TUNER_PANEL_ACTIVE);
    draw_rect(fb, fb_width, fb_height, b->x, b->y, b->w, b->h, panel_color);
    draw_borders(fb, fb_width, fb_height, b);

    // Render Value Adjusters (Tuners w floats, uints or bools)
    if (b->target_value) {
        // Center the tuner split line and offset it based on value
        int mid = b->x + b->w / 2;
        int thickness = (int)get_param_float(PARAM_UI_TUNER_SPLIT_LINE_THICKNESS);

        float current_value = 0.0f;

        switch (b->type)
        {
            case PARAM_TYPE_FLOAT:
                current_value = *(float*)b->target_value;
                break;

            case PARAM_TYPE_BOOL:
                current_value = *(bool*)b->target_value;
                break;

            case PARAM_TYPE_ENUM:
                current_value = *(int*)b->target_value;
                break;
        }

        int offset = (int)(((current_value - b->min_value) * b->w) /
                           (b->max_value - b->min_value)) - b->w / 2;

        int mid_adjusted = mid + offset - (thickness - 1) / 2;

        // Keep split line inside tuner bounds
        int min_x = b->x;
        int max_x = b->x + b->w - thickness;

        if (mid_adjusted < min_x)
            mid_adjusted = min_x;
        else if (mid_adjusted > max_x)
            mid_adjusted = max_x;

        draw_panel(
            fb,
            fb_width,
            fb_height,
            mid_adjusted,
            b->y + 4,
            thickness,
            b->h - 8,
            get_param_color(PARAM_UI_COLOR_TUNER_SPLIT_LINE),
            get_param_color(PARAM_UI_COLOR_TUNER_SPLIT_LINE_BORDER));
        char val[32];
        switch (b->type)
        {
            case PARAM_TYPE_BOOL:
                bool bval = *(bool*)b->target_value;
                snprintf(val, sizeof(val), "%s", bval ? "ON" : "OFF");
                break;
            case PARAM_TYPE_ENUM:
            {
                int eval = *(int*)b->target_value;

                if (b->enum_definition)
                {
                    for (uint32_t i = 0; i < b->enum_definition->count; i++)
                    {
                        if (b->enum_definition->entries[i].value == eval)
                        {
                            snprintf(
                                val,
                                sizeof(val),
                                "%s",
                                b->enum_definition->entries[i].name
                            );
                            break;
                        }
                    }
                }

                break;
            }
            case PARAM_TYPE_FLOAT:
                snprintf(
                    val,
                    sizeof(val),
                    "%.6f",
                    *(float*)b->target_value
                );
                break;
        }
        draw_text(fb, fb_width, fb_height, val, b->x + 20, b->y + b->h - 16, get_param_color(PARAM_UI_COLOR_TUNER_VALUE_TEXT), 2, b);
    }

    //  Render Interactive Text Fields / Scrollable Elements
    if (b->is_editable && b->editable_content) {
        draw_editor(b);
        if (b->content_height > visible_h) draw_scrollbar(b, fb, fb_width, fb_height);
        if (b->content_width > visible_w)  draw_horizontal_scrollbar(b, fb, fb_width, fb_height);
    } 
    else if (b->content && b->content[0]) {
        int txt_pad = 6;
        int scale = calculate_text_scale(b->content, b->w - (txt_pad * 2), b->h - (txt_pad * 2), 3, b->is_scrollable);
        int line_h = get_param_float(PARAM_UI_FONT_HEIGHT) * scale + 8;

        if (b->is_scrollable) {
            if (b->content_height <= 0.0f) {
                int num_lines = 1;
                for (const char* p = b->content; *p; ++p) if (*p == '\n') num_lines++;
                b->content_height = (float)num_lines * line_h;
            }
            b->scroll_offset = fmaxf(0.0f, fminf(b->scroll_offset, fmaxf(0.0f, b->content_height - (float)(b->h - 2 * txt_pad))));
            b->scroll_offset_x = fmaxf(0.0f, fminf(b->scroll_offset_x, fmaxf(0.0f, b->content_width - (float)(b->w - 2 * txt_pad))));

            draw_multiline_text(fb, fb_width, fb_height, b->x + txt_pad - (int)b->scroll_offset_x, b->y + txt_pad - (int)b->scroll_offset,
                                b->content, get_param_color(PARAM_UI_COLOR_MAIN_TEXT), scale, line_h, b);

            if (b->content_height > (float)(b->h - 2 * txt_pad)) draw_scrollbar(b, fb, fb_width, fb_height);
            if (b->content_width > (float)(b->w - 2 * txt_pad))  draw_horizontal_scrollbar(b, fb, fb_width, fb_height);
        } else {
            draw_multiline_text(fb, fb_width, fb_height, b->x + txt_pad, b->y + txt_pad, b->content, get_param_color(PARAM_UI_COLOR_MAIN_TEXT), scale, line_h, b);
        }
    }
}
