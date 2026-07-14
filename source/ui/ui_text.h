#pragma once
#include "core/project.h"
#include "core/string.h"
#include "ui/ui.h"
#include "ui/ui_button.h"
#include "ui/ui_callbacks.h"
#include "core/params/params.h"
#include "ui/ui_editor.h"
#include "ui/ui_tuner.h"
#include "ui/ui_mode.h"

void ui_add_scrollable_text(UI_Context* ctx, int x, int y, int w, int h, const char* text);
void ui_add_scrollable_text_editor(UI_Context* ctx, int x, int y, int w, int h, const char* initial_text, const char* filepath);