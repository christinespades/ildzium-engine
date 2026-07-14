#pragma once
#include "core/string.h"
#include "ui.h"
#include "core/params/params.h"
#include "ui/ui_editor_clipboard.h"

typedef struct UI_Button UI_Button;
void ensure_capacity(UI_Button* b, size_t needed);
void move_cursor_left(UI_Button* b, int steps);
void move_cursor_right(UI_Button* b, int steps);
void move_cursor_vertical(UI_Button* b, int direction);
void move_to_home(UI_Button* b);
void move_to_end(UI_Button* b);
int get_text_length(UI_Button* b);
int get_char_index_from_mouse(UI_Button* b);
void select_word_at_position(UI_Button* b, int click_pos);
void select_all_text(UI_Button* b);
void cut_selection_to_clipboard(UI_Button* b);