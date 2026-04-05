#pragma once
#include "ui.h"

void ensure_capacity(UI_Button* b, size_t needed);
void insert_char_at_cursor(UI_Button* b, char c);
void delete_char_before_cursor(UI_Button* b);
void delete_char_at_cursor(UI_Button* b);
void move_cursor_left(UI_Button* b, int steps);
void move_cursor_right(UI_Button* b, int steps);
void move_to_home(UI_Button* b);
void move_to_end(UI_Button* b);
int get_text_length(UI_Button* b);
void copy_selection_to_clipboard(UI_Button* b);
void paste_from_clipboard(UI_Button* b);
int get_char_index_from_mouse(UI_Button* b, int mouse_x, int mouse_y);
void select_word_at_position(UI_Button* b, int click_pos);