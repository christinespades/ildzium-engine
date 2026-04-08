#include "ui/ui_draw_text_rect.h"
#include "ui/ui_params.h"
#include "core/fonts.h"

void draw_pixel(uint32_t* fb, int fb_w, int fb_h, int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= fb_w || y >= fb_h) return;
    fb[y * fb_w + x] = color;
}

void draw_rect(uint32_t* fb, int fb_w, int fb_h, int x, int y, int w, int h, uint32_t color)
{
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            draw_pixel(fb, fb_w, fb_h, x+i, y+j, color);
}

// Generic function to draw a panel with background and border
void ui_draw_panel(uint32_t* fb, int fb_width, int fb_height,
                   int x, int y, int w, int h,
                   uint32_t bg_color, uint32_t border_color)
{
    // Background
    draw_rect(fb, fb_width, fb_height, x, y, w, h, bg_color);

    // Border (thickness = 2 pixels)
    draw_rect(fb, fb_width, fb_height, x, y, w, 2, border_color);                    // top
    draw_rect(fb, fb_width, fb_height, x, y + h - 2, w, 2, border_color);            // bottom
    draw_rect(fb, fb_width, fb_height, x, y, 2, h, border_color);                    // left
    draw_rect(fb, fb_width, fb_height, x + w - 2, y, 2, h, border_color);            // right
}

void draw_char(uint32_t* fb, int fb_w, int fb_h, char c, int x, int y, uint32_t color, int scale) {
    if ((unsigned char)c >= 128) return;
    if (scale < 1) scale = 1;

    for (int row = 0; row < 8; row++) {
        uint8_t bits = font8x8_basic[(unsigned char)c][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << (7 - col))) {
                // Draw scaled pixel block
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        draw_pixel(fb, fb_w, fb_h, x + col*scale + sx, y + row*scale + sy, color);
                    }
                }
            }
        }
    }
}

void draw_text(uint32_t* fb, int fb_w, int fb_h,
               const char* text, int x, int y,
               uint32_t normal_color, int scale,
               UI_Button* b)
{
    if (!text || y >= fb_h || x >= fb_w) return;

    bool has_selection = (b && b->is_editable && b->selection_start != -1);
    int sel_start = has_selection ? 
        (b->selection_start < b->selection_end ? b->selection_start : b->selection_end) : -1;
    int sel_end   = has_selection ? 
        (b->selection_start > b->selection_end ? b->selection_start : b->selection_end) : -1;

    int cursor_x = x;
    int cursor_y = y;
    int char_w = FONT_WIDTH * scale;
    int char_h = FONT_HEIGHT * scale;
    int global_char_idx = 0;

    while (*text)
    {
        if (*text == '\n')
        {
            cursor_y += FONT_HEIGHT * scale;
            cursor_x = x;
            text++;
            global_char_idx++;
            if (cursor_y >= fb_h) break;
            continue;
        }

        uint32_t draw_color = normal_color;

        // Selected text = white
        if (has_selection && global_char_idx >= sel_start && global_char_idx < sel_end)
        {
            draw_color = COLOR_SELECTED_TEXT;
        }

        if (cursor_x + char_w > 0 && cursor_y + char_h > 0 &&
            cursor_x < fb_w && cursor_y < fb_h)
        {
            draw_char(fb, fb_w, fb_h, *text, cursor_x, cursor_y, draw_color, scale);
        }

        cursor_x += char_w;
        if (cursor_x > fb_w) break;

        text++;
        global_char_idx++;
    }
}

void draw_multiline_text(uint32_t* fb, int fb_w, int fb_h,
                         int x, int y, const char* text,
                         uint32_t color, int base_scale, int line_height, UI_Button* b)
{
    if (!text || !*text) return;

    char line[512];
    const char* p = text;
    int current_y = y;
    int char_h = FONT_HEIGHT * base_scale;

    while (*p)
    {
        // Early exit if this line is completely below the framebuffer
        if (current_y >= fb_h) break;

        int i = 0;
        while (*p && *p != '\n' && i < 510)
        {
            line[i++] = *p++;
        }
        line[i] = '\0';

        if (i > 0)
        {
            int draw_scale = base_scale;
            int offset_x = 0;
            const char* lines_to_draw = line;

            // Header control character support
            if (line[0] >= 0x02 && line[0] <= 0x07)
            {
                int level = line[0] - 0x01;
                draw_scale = (level == 1) ? 2 : (level <= 3 ? 1 : base_scale);
                lines_to_draw = line + 1;
                offset_x = 4;
            }

            // Only draw if the line is at least partially visible
            if (current_y + char_h > 0)
            {
                draw_text(fb, fb_w, fb_h,
                          lines_to_draw,
                          x + offset_x,
                          current_y,
                          color,
                          draw_scale, b);
            }
        }

        current_y += line_height;
        if (*p == '\n') p++;
    }
}