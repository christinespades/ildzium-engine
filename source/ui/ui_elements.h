#pragma once
#include "core/debug.h"
#include "core/project.h"
#include "core/string.h"
#include "core/window.h"
#include "ui/ui.h"
#include "ui/ui_callbacks.h"
#include "ui/ui_params.h"
#include "ui/ui_editor.h"
#include "ui/ui_tuner.h"
#include "ui/modes/ui_project.h"

typedef void (*UI_ButtonCallback)(void);
void ui_add_button(UI_Context* ctx, int x, int y, int w, int h, const char* text,
                   UI_ButtonCallback on_click,
                   UI_ButtonCallback on_held,
                   UI_ButtonCallback on_release,
                   UI_ButtonCallback on_hover,
                   const char* tooltip_text);

void ui_add_scrollable_text(UI_Context* ctx, int x, int y, int w, int h, const char* text);
void ui_add_scrollable_text_editor(UI_Context* ctx, int x, int y, int w, int h, const char* initial_text, const char* filepath);

extern bool g_show_project_modal;
extern bool g_is_rename_mode;
extern char g_modal_old_name[MAX_PROJECT_NAME];

void setup_project_controls(UI_Context* ctx);
void project_on_load_clicked(void);
void project_on_delete_clicked(void);
void project_on_create_btn_clicked(void);
void project_on_rename_btn_clicked(void);