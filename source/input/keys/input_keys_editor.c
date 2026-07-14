#include "pch.h"
#include "input/keys/input_keys_editor.h"

void handle_editor_key(platform_key key)
{
    int mods = platform_get_mods();
    int shift = (mods & MOD_SHIFT);
    int ctrl  = (mods & MOD_CTRL);

    for (int i = 0; i < g_ui_ctx->button_count; i++) {
        UI_Button* b = &g_ui_ctx->buttons[i];
        if (!b->is_editable) continue;
        if (i != g_ui_ctx->active_button_index) continue;
        LOGI("Handling editor key for button %s with index %u", b->content, g_ui_ctx->active_button_index);

        if (key == KEY_BACKSPACE) {
            delete_char_before_cursor(b);
        }
        else if (key == KEY_DELETE) {
            delete_char_at_cursor(b);
        }
        else if (key == KEY_LEFT) {
            int steps = ctrl ? 5 : 1;
            move_cursor_left(b, steps);

            if (shift) {
                if (b->selection_start == -1)
                    b->selection_start = b->cursor_pos + steps;
                b->selection_end = b->cursor_pos;
            } else {
                b->selection_start = b->selection_end = -1;
            }
        }
        else if (key == KEY_RIGHT) {
            int steps = ctrl ? 5 : 1;
            move_cursor_right(b, steps);

            if (shift) {
                if (b->selection_start == -1)
                    b->selection_start = b->cursor_pos - steps;
                b->selection_end = b->cursor_pos;
            } else {
                b->selection_start = b->selection_end = -1;
            }
        }
        else if (key == KEY_UP) {
            move_cursor_vertical(b, -1);
        }
        else if (key == KEY_DOWN) {
            move_cursor_vertical(b, +1);
        }
        else if (key == KEY_HOME) {
            move_to_home(b);
            if (!shift) b->selection_start = b->selection_end = -1;
            else if (b->selection_start == -1)
                b->selection_start = b->cursor_pos;
        }
        else if (key == KEY_END) {
            move_to_end(b);
            if (!shift) b->selection_start = b->selection_end = -1;
        }
        else if (key == KEY_ENTER || key == KEY_KP_ENTER) {
            if (b->is_typing)
            {
                LOGI("Pressed Enter inside a editable tuner");
                float value=(float)atof(b->editable_content);

                switch (b->type)
                {
                    case PARAM_TYPE_FLOAT:
                        *(float*)b->target_value = value;
                        break;

                    case PARAM_TYPE_ENUM:
                        *(int*)b->target_value = (int)value;
                        break;
                }

                b->is_typing=false;
                b->click_count = 0;
                return;
            }
            insert_char_at_cursor(b, '\n');
        }
        else if (ctrl && key == KEY_S) {
            save_file(b->filepath, b->editable_content);
        }
        else if (ctrl && key == KEY_C) {
            copy_selection_to_clipboard(b);
        }
        else if (ctrl && key == KEY_X) {
            cut_selection_to_clipboard(b);
        }
        else if (ctrl && key == KEY_A) {
            select_all_text(b);
        }
        else if (ctrl && key == KEY_V) {
            paste_from_clipboard(b);
        }
        else if (ctrl && key == KEY_Z) {
            perform_undo(b);
            return;
        }
        else if (ctrl && key == KEY_Y) {
            perform_redo(b);
            return;
        }

        return;
    }
}