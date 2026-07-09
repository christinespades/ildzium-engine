// ui/modes/ui_project.c
#include "pch.h"
#include "ui/ui.h"
#include "ui/ui_elements.h"
#include "../../core/project.h"
#include <stdio.h>
#include <string.h>

void project_on_load_clicked()
{
    UI_Button* b = NULL;
    project_load(b->content);
    setup_project_controls(g_ui_ctx);
}

void project_on_delete_clicked()
{
	UI_Button* b = NULL;
    if (strcmp(b->content, "Default") == 0) 
        return;
    
    project_delete(b->content);
    setup_project_controls(g_ui_ctx);
}

void project_on_create_btn_clicked()
{
    project_show_new_dialog();
    setup_project_controls(g_ui_ctx);
}

void project_on_rename_btn_clicked()
{
	UI_Button* b = NULL;
    project_show_rename_dialog(b->content);
    setup_project_controls(g_ui_ctx);
}

void project_modal_on_ok()
{
    UI_Context* ctx = g_ui_ctx;
    UI_Button* editor = NULL;

    for (int i = ctx->button_count - 1; i >= 0; i--)
    {
        if (ctx->buttons[i].is_editable && ctx->buttons[i].editable_content)
        {
            editor = &ctx->buttons[i];
            break;
        }
    }

    if (editor && editor->editable_content && editor->editable_content[0] != '\0')
    {
        const char* name = editor->editable_content;

        if (g_is_rename_mode)
            project_rename(g_modal_old_name, name);
        else
            project_create(name);
    }

    g_show_project_modal = false;
    setup_project_controls(ctx);
}

void project_modal_on_cancel()
{
    g_show_project_modal = false;
    setup_project_controls(g_ui_ctx);
}

void draw_project_modal(UI_Context* ctx)
{
    const int modal_w = 520;
    const int modal_h = 220;
    const int modal_x = (g_width - modal_w) / 2;
    const int modal_y = (g_height - modal_h) / 2;

    // Background
    ui_add_button(ctx, modal_x, modal_y, modal_w, modal_h, "", NULL, NULL, NULL, NULL, NULL);

    const char* title = g_is_rename_mode ? "Rename Project" : "Create New Project";
    ui_add_button(ctx, modal_x + 20, modal_y + 20, modal_w - 40, 35, title, NULL, NULL, NULL, NULL, NULL);

    // Text Editor
    char initial[128] = "";
    if (g_is_rename_mode)
        strncpy(initial, g_modal_old_name, sizeof(initial) - 1);

    ui_add_scrollable_text_editor(ctx, modal_x + 40, modal_y + 75, 
                                  modal_w - 80, 55, initial, "modal_project_name");

    // OK / Cancel
    ui_add_button(ctx, modal_x + 60, modal_y + 150, 160, 45, "OK",
                  (UI_ButtonCallback)project_modal_on_ok, NULL, NULL, NULL, NULL);

    ui_add_button(ctx, modal_x + 280, modal_y + 150, 160, 45, "Cancel",
                  (UI_ButtonCallback)project_modal_on_cancel, NULL, NULL, NULL, NULL);
}

void setup_project_controls(UI_Context* ctx)
{
    const int btn_h = 38;
    const int spacing = 10;
    int y = 60;

    ui_add_button(ctx, 40, y, 420, 40, "PROJECT MANAGER", NULL, NULL, NULL, NULL, NULL);

    int proj_count;
    Project* projects = project_get_list(&proj_count);

    for (int i = 0; i < proj_count; i++)
    {
        ui_add_button(ctx, 50, y, 340, btn_h, projects[i].name,
                      (UI_ButtonCallback)project_on_load_clicked, NULL, NULL, NULL, NULL);

        ui_add_button(ctx, 400, y, 70, btn_h, "Rename",
                      (UI_ButtonCallback)project_on_rename_btn_clicked, NULL, NULL, NULL, NULL);

        ui_add_button(ctx, 480, y, 70, btn_h, "Delete",
                      (UI_ButtonCallback)project_on_delete_clicked, NULL, NULL, NULL, NULL);

        y += btn_h + spacing;
    }

    ui_add_button(ctx, 40, y + btn_h + spacing, 260, 50, "Create New Project",
                  (UI_ButtonCallback)project_on_create_btn_clicked, NULL, NULL, NULL, NULL);

    if (g_show_project_modal)
        draw_project_modal(ctx);
}