#include "pch.h"
#include "ui_calculations.h"

int calculate_text_scale(const char* text, int max_w, int max_h, int max_target_scale, bool is_scrollable)
{
    if (!text || text[0] == '\0' || max_w <= 0 || max_h <= 0 || max_target_scale < 1)
        return 1;

    int max_chars_in_line = 0;
    int current_chars = 0;
    int num_lines = 1;
    int max_word_len = 0;
    int current_word_len = 0;

    for (const char* p = text; *p; ++p) {
        if (*p == '\n') {
            if (current_chars > max_chars_in_line) max_chars_in_line = current_chars;
            current_chars = 0;
            num_lines++;
        } else {
            current_chars++;
        }

        if (*p == ' ' || *p == '\t' || *p == '\n') {
            if (current_word_len > max_word_len) max_word_len = current_word_len;
            current_word_len = 0;
        } else {
            current_word_len++;
        }
    }

    if (current_chars > max_chars_in_line) max_chars_in_line = current_chars;
    if (current_word_len > max_word_len) max_word_len = current_word_len;

    float font_w = get_param_float(PARAM_UI_FONT_WIDTH);
    float font_h = get_param_float(PARAM_UI_FONT_HEIGHT);

    float base_char_w = (max_word_len > 0 ? (float)max_word_len : (float)max_chars_in_line) * font_w;
    float base_line_h = font_h + 8.0f;
    float base_text_h = (float)num_lines * base_line_h;

    int guess_w = (int)(max_w / (base_char_w > 0.0f ? base_char_w : 1.0f));
    int guess_h = (int)(max_h / (base_text_h > 0.0f ? base_text_h : 1.0f));

    int scale = max_target_scale;
    if (!is_scrollable) {
        scale = guess_w < guess_h ? guess_w : guess_h;
    } else {
        scale = guess_w;
    }

    if (scale > max_target_scale) scale = max_target_scale;
    if (scale < 1) scale = 1;

    for (int s = scale; s >= 1; --s) {
        float text_w = (float)max_chars_in_line * font_w * (float)s;

        if (is_scrollable) {
            if (text_w <= (float)max_w) return s;
        } else {
            float text_h = (float)num_lines * (font_h * (float)s + 8.0f);
            if (text_w <= (float)max_w && text_h <= (float)max_h) return s;
        }
    }

    return 1;
}