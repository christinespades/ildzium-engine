#include "pch.h"
#include "ui_update_editable_button.h"

void ui_update_editable_button(UI_Button* b, int mouse_pressed, int mouse_pressed_this_frame)
{
    // === TUNER OVERRIDE ===
    //LOGI("Clicked on an editable button");
    if (b->target_value != NULL) {
        LOGI("Clicked on tuner");
        double now = ildz_get_time();
        int pos = get_char_index_from_mouse(b);

        // Track double click to engage text mode
        if (mouse_pressed_this_frame) {
            if (now - b->last_click_time < 0.35 && pos == b->last_click_pos) {
                b->click_count++;
            } else {
                b->click_count = 1;
            }
            b->last_click_time = now;
            b->last_click_pos = pos;

            // DOUBLE CLICK: Engage text editing mode and select the text
            if (b->click_count == 2) {
                LOGI("Double clicked on tuner");
                //select_word_at_position(b, pos);
                b->cursor_pos = b->selection_end;
                
                if (ui_button_has_focus(b))
                {
                    b->is_typing = true;
                    b->click_count = 0;
                    LOGI("Tuner button ready for typing");
                }
            }
        }
        return;
    }
    //b->is_typing = false;

    int pos = get_char_index_from_mouse(b);

    if (mouse_pressed_this_frame) {
        b->drag_start_pos = pos;

        // --- 1. INITIAL CLICK FRAME ---
        double now = ildz_get_time();

        // === SHIFT + CLICK ===
        if (platform_get_key_down(KEY_LEFT_SHIFT) || platform_get_key_down(KEY_RIGHT_SHIFT)) {
            if (b->selection_start == -1) {
                b->selection_start = b->cursor_pos;
            }
            b->selection_end = pos;
            b->cursor_pos = pos;
        }
        // === NORMAL CLICK (Single / Double / Triple) ===
        else {
            if (now - b->last_click_time < 0.35 && pos == b->last_click_pos) {
                b->click_count++;
            } else {
                b->click_count = 1;
            }

            b->last_click_time = now;
            b->last_click_pos = pos;

            if (b->click_count == 1) {
                // Single click: move caret, reset selection to a starting point
                b->cursor_pos = pos;
                b->selection_start = -1;
                b->selection_end = -1;
            }
            else if (b->click_count == 2) {
                select_word_at_position(b, pos);
                b->cursor_pos = b->selection_end;
            }
            else if (b->click_count >= 3) {
                select_entire_line(b, pos);
                b->click_count = 0;
            }
        }
    }

    // --- 2. DRAGGING / HOLDING FRAMES ---
    if (mouse_pressed && b->is_dragging) {
        // Shift-drag or regular drag on a single-click sequence updates selection
        if (platform_get_key_down(KEY_LEFT_SHIFT) || platform_get_key_down(KEY_RIGHT_SHIFT)) {
            b->selection_end = pos;
            b->cursor_pos = pos;
        }
        else if (b->click_count == 1) {
            // Regular click-and-drag selection logic
            if (pos != b->drag_start_pos) {
                b->selection_start = b->drag_start_pos;
                b->selection_end = pos;
            } else {
                // If the user drags back to the start, clear selection visualizer
                b->selection_start = -1;
                b->selection_end = -1;
            }
            b->cursor_pos = pos;
        }
    }

    if (!mouse_pressed && b->is_dragging) {
        ui_button_mouse_up(b); // sets b->is_dragging = false
    }

    b->content_height = 0.0f;
    // Important: Do NOT return here for editable buttons!
    // We still want to run the general button logic below if needed
    // Only skip if you have a good reason
}
