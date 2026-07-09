#include "pch.h"
#include "ui/ui_draw_scrollbar.h"

void draw_scrollbar_generic(uint32_t* fb, int fb_w, int fb_h, 
                            int track_x, int track_y, int track_w, int track_h,
                            float content_dim, float visible_dim, float scroll_offset, 
                            bool is_horizontal) 
{
    if (content_dim <= visible_dim) return;

    float scroll_ratio = visible_dim / content_dim;
    float thumb_dim = fmaxf(30.0f, visible_dim * scroll_ratio);
    float max_scroll = content_dim - visible_dim;
    float progress = (max_scroll > 0) ? (scroll_offset / max_scroll) : 0.0f;
    int progress_offset = (int)(progress * (visible_dim - thumb_dim));

    // Calculate dynamic thumb positioning based on orientation axis
    int thumb_x = track_x + (is_horizontal ? progress_offset : 0);
    int thumb_y = track_y + (is_horizontal ? 0 : progress_offset);
    int thumb_w = is_horizontal ? (int)thumb_dim : track_w;
    int thumb_h = is_horizontal ? track_h : (int)thumb_dim;

    // 1. Track border & Track
    draw_rect(fb, fb_w, fb_h, track_x - 1, track_y - 1, track_w + 2, track_h + 2, get_param_color(PARAM_UI_COLOR_SCROLLBAR_TRACK_BORDER));
    draw_rect(fb, fb_w, fb_h, track_x, track_y, track_w, track_h, get_param_color(PARAM_UI_COLOR_SCROLLBAR_BG));

    // 2. Thumb border & Thumb
    draw_rect(fb, fb_w, fb_h, thumb_x - 1, thumb_y - 1, thumb_w + 2, thumb_h + 2, get_param_color(PARAM_UI_COLOR_SCROLLBAR_THUMB_BORDER));
    draw_rect(fb, fb_w, fb_h, thumb_x, thumb_y, thumb_w, thumb_h, get_param_color(PARAM_UI_COLOR_SCROLLBAR_THUMB));
}

void draw_scrollbar(UI_Button* b, uint32_t* fb, int fb_width, int fb_height) {
    if (!b->is_scrollable) return;

    // Get vertical scrollbar dimensions/paddings
    float pad_l = get_param_float(PARAM_UI_SCROLLBAR_V_PAD_LEFT);
    float pad_r = get_param_float(PARAM_UI_SCROLLBAR_V_PAD_RIGHT);
    float pad_t = get_param_float(PARAM_UI_SCROLLBAR_V_PAD_TOP);
    float pad_b = get_param_float(PARAM_UI_SCROLLBAR_V_PAD_BOTTOM);
    int bar_w = (int)get_param_float(PARAM_UI_SCROLLBAR_V_THICKNESS);

    // Calculate vertical track constraints
    int bar_x = b->x + b->w - bar_w - (int)pad_r + (int)pad_l;
    int bar_y = b->y + (int)pad_t;
    float visible_h = (float)(b->h - (int)pad_t - (int)pad_b);

    draw_scrollbar_generic(fb, fb_width, fb_height,
                           bar_x, bar_y, bar_w, (int)visible_h,
                           b->content_height, visible_h, b->scroll_offset, false);
}

void draw_horizontal_scrollbar(UI_Button* b, uint32_t* fb, int fb_width, int fb_height) {
    if (!b->is_scrollable) return;

    // Get horizontal scrollbar dimensions/paddings
    float pad_l = get_param_float(PARAM_UI_SCROLLBAR_H_PAD_LEFT);
    float pad_r = get_param_float(PARAM_UI_SCROLLBAR_H_PAD_RIGHT);
    float pad_t = get_param_float(PARAM_UI_SCROLLBAR_H_PAD_TOP);
    float pad_b = get_param_float(PARAM_UI_SCROLLBAR_H_PAD_BOTTOM);
    int bar_h = (int)get_param_float(PARAM_UI_SCROLLBAR_H_THICKNESS);

    // Calculate horizontal track constraints
    int bar_x = b->x + (int)pad_l;
    int bar_y = b->y + b->h - bar_h - (int)pad_b + (int)pad_t;
    float visible_w = (float)(b->w - (int)pad_l - (int)pad_r);

    draw_scrollbar_generic(fb, fb_width, fb_height,
                           bar_x, bar_y, (int)visible_w, bar_h,
                           b->content_width, visible_w, b->scroll_offset_x, true);
}