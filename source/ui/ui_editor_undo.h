#pragma once
#include "ui/ui_button.h"
#include "ui_editor.h"
#include "ui/ui_editor_state.h"
#include "ui/ui_params.h"

void free_editor_state(EditorState* s);

// must be called when button is destroyed
void cleanup_editor_undo(UI_Button* b);
void copy_editor_state(EditorState* dst, const UI_Button* b);
void push_undo_state(UI_Button* b);
void restore_state(UI_Button* b, EditorState* s);
void init_editor_undo(UI_Button* b, int max_steps);
void perform_undo(UI_Button* b);
void perform_redo(UI_Button* b);