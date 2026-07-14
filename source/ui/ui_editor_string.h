#pragma once
#include "ui/ui_editor_undo.h"

typedef struct UI_Button UI_Button;
void insert_char_at_cursor(UI_Button* b, char c);
void insert_string_at_cursor(UI_Button* b, const char* text);
void delete_char_before_cursor(UI_Button* b);
void delete_char_at_cursor(UI_Button* b);