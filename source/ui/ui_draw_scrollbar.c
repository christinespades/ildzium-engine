#include "pch.h"
#include "ui/ui_draw_scrollbar.h"

void draw_scrollbar(UI_Button* b, uint32_t* fb, int fb_width, int fb_height) {
    if (!b->is_scrollable || b->content_height <= b->h) return;

    int padding = 12;
    float visible_height = (float)(b->h - 2 * padding);
    float scroll_ratio = visible_height / b->content_height;
    float thumb_height = fmaxf(30.0f, visible_height * scroll_ratio);
    float max_scroll = fmaxf(0.0f, b->content_height - visible_height);
    float progress = (max_scroll > 0) ? (b->scroll_offset / max_scroll) : 0.0f;

    int scrollbar_x = b->x + b->w - 14;
    int scrollbar_y = b->y + padding + (int)(progress * (visible_height - thumb_height));

    int track_y = b->y + padding;
    int track_h = (int)visible_height;

    // --- Track border ---
    draw_rect(fb, fb_width, fb_height,
              scrollbar_x - 1, track_y - 1,
              8 + 2, track_h + 2,
              COLOR_BORDER);

    // Track
    draw_rect(fb, fb_width, fb_height,
              scrollbar_x, track_y,
              8, track_h,
              COLOR_SCROLLBAR_BG);

    // --- Thumb border ---
    draw_rect(fb, fb_width, fb_height,
              scrollbar_x - 1, scrollbar_y - 1,
              8 + 2, (int)thumb_height + 2,
              COLOR_BORDER);

    // Thumb
    draw_rect(fb, fb_width, fb_height,
              scrollbar_x, scrollbar_y,
              8, (int)thumb_height,
              COLOR_SCROLLBAR_THUMB);
}