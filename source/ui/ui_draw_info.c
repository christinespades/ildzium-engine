#include "ui_draw_info.h"
#include <string.h>
#include <stdlib.h>
#include "core/settings.h"
#include "ui/ui_draw.h"
#include "ui/ui_draw_text_rect.h"
#include "ui/ui_params.h"

static float info_scroll_x = 0.0f;   // persistent horizontal scroll offset

// ====================== TEXT WIDTH HELPER ======================
// Simple monospaced font width calculation (assumes your font is fixed-width)
int get_text_width(const char* text, int scale)
{
    if (!text) return 0;
    int len = 0;
    while (text[len]) len++;
    return len * FONT_WIDTH * scale;   // FONT_WIDTH should be defined in your font header (usually 8 or 9)
}

// ====================== HORIZONTAL SCROLLING TEXT ======================
void draw_text_hscroll(uint32_t* fb, int fb_width, int fb_height,
                       const char* text,
                       int x, int y,
                       uint32_t color, int scale,
                       int scroll_offset_x,      // how much to shift left (positive = scroll right)
                       int clip_width)           // maximum width we are allowed to draw into
{
    if (!text || clip_width <= 0) return;

    int char_w = FONT_WIDTH * scale;
    int start_x = x - scroll_offset_x;   // negative offset moves text to the left

    int i = 0;
    while (text[i])
    {
        int char_screen_x = start_x + i * char_w;

        // Only draw characters that are inside the clip region
        if (char_screen_x + char_w > x && char_screen_x < x + clip_width)
        {
            draw_char(fb, fb_width, fb_height, text[i], char_screen_x, y, color, scale);
        }

        // Stop early if we've gone past the clip area
        if (char_screen_x > x + clip_width)
            break;

        i++;
    }
}

void ui_draw_info(uint32_t* fb, int fb_width, int fb_height, float dt)
{
    if (!(g_renderer_flags & RENDERER_SHOW_FPS))
        return;

    // === Build info string safely ===
    char info_text[512];  // increased size for safety

    // === Panel setup ===
    const int panel_height = 32;
    const int panel_y      = fb_height - panel_height;
    const int padding      = 20;

    ui_draw_panel(fb, fb_width, fb_height,
                  0, panel_y, fb_width, panel_height,
                  COLOR_BOTTOM_INFO_PANEL_BG, COLOR_BOTTOM_INFO_PANEL_BORDER);

    // === Horizontal scrolling text ===
    int text_y = panel_y + (panel_height - FONT_HEIGHT) / 2;

    // Auto-scroll when text is wider than available space
    int available_width = fb_width - 2 * padding;

    int text_pixel_width = get_text_width(info_text, 1);  // scale = 1

    // Auto-scroll logic
    info_scroll_x += BOTTOM_INFO_PANEL_AUTOSCROLL_SPEED * dt;

    // Loop the scroll (seamless)
    if (info_scroll_x > text_pixel_width + 50)   // extra gap
        info_scroll_x = -100.0f;                 // start from left with small gap

    // Draw with horizontal offset (clipped by draw function)
    draw_text_hscroll(fb, fb_width, fb_height,
                      info_text,
                      padding, text_y,
                      COLOR_BOTTOM_INFO_PANEL_TEXT, 1,
                      (int)info_scroll_x,
                      available_width);
}