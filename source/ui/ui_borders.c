#include "pch.h"
#include "ui_borders.h"

void draw_borders(uint32_t* fb, int fb_width, int fb_height, UI_Button* b)
{
    uint32_t outer_color = get_param_color(PARAM_UI_COLOR_BORDER_FALLBACK);
    uint32_t inner_color = get_param_color(PARAM_UI_COLOR_BORDER_FALLBACK);
    int outer_thick = 2;
    int inner_thick = 2;

    // Determine border styling based on the type of button
    if (b->target_value) {
        // Tuner button styling
        outer_color = get_param_color(PARAM_UI_COLOR_TUNER_BORDER_OUTER);
        inner_color = get_param_color(PARAM_UI_COLOR_TUNER_BORDER_INNER);
        outer_thick = 2;
        inner_thick = 1;
    } 
    else if (b->is_editable) {
        // Editable text area styling
        outer_color = get_param_color(PARAM_UI_COLOR_EDIT_BORDER_OUTER);
        inner_color = get_param_color(PARAM_UI_COLOR_EDIT_BORDER_INNER);
        outer_thick = 3;
        inner_thick = 2;
    } 
    else if (b->is_scrollable) {
        // Scrollable text box styling
        outer_color = get_param_color(PARAM_UI_COLOR_SCROLL_BORDER_OUTER);
        inner_color = get_param_color(PARAM_UI_COLOR_SCROLL_BORDER_INNER);
        outer_thick = 2;
        inner_thick = 2;
    } 
    else {
        // Default standard button styling
        outer_color = get_param_color(PARAM_UI_COLOR_DEFAULT_BORDER_OUTER);
        inner_color = get_param_color(PARAM_UI_COLOR_DEFAULT_BORDER_INNER);
        outer_thick = 2;
        inner_thick = 0; // Set to 0 if you don't want a dual border on basic buttons
    }

    // Track active rendering rectangle dimensions
    int rx = b->x;
    int ry = b->y;
    int rw = b->w;
    int rh = b->h;

    // --- 1. Draw Outer Border ---
    if (outer_thick > 0) {
        draw_rect(fb, fb_width, fb_height, rx, ry, rw, outer_thick, outer_color);                 // Top
        draw_rect(fb, fb_width, fb_height, rx, ry + rh - outer_thick, rw, outer_thick, outer_color); // Bottom
        draw_rect(fb, fb_width, fb_height, rx, ry, outer_thick, rh, outer_color);                 // Left
        draw_rect(fb, fb_width, fb_height, rx + rw - outer_thick, ry, outer_thick, rh, outer_color); // Right

        // Shrink geometry inwards to make room for the inner border
        rx += outer_thick;
        ry += outer_thick;
        rw -= (outer_thick * 2);
        rh -= (outer_thick * 2);
    }

    // --- 2. Draw Inner Border ---
    if (inner_thick > 0 && rw > 0 && rh > 0) {
        draw_rect(fb, fb_width, fb_height, rx, ry, rw, inner_thick, inner_color);                 // Top
        draw_rect(fb, fb_width, fb_height, rx, ry + rh - inner_thick, rw, inner_thick, inner_color); // Bottom
        draw_rect(fb, fb_width, fb_height, rx, ry, inner_thick, rh, inner_color);                 // Left
        draw_rect(fb, fb_width, fb_height, rx + rw - inner_thick, ry, inner_thick, rh, inner_color); // Right
    }
}