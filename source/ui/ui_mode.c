#include "pch.h"
#include "ui/ui_mode.h"
#include "ui/ui_button.h"

#define UI_PARAM_TUNER_ENUM(type, id, name_str, value, enum_def, tooltip) \
    ui_add_tuner_enum(ctx, l.current_x, ui_layout_next_y(&l), l.btn_w, l.btn_h, name_str, PARAM_##id, enum_def, tooltip);

#define UI_PARAM_TUNER(type, id, name_str, min_val, max_val, tooltip) \
    if (#type[0] == 'b')                                               \
    {                                                                  \
        ui_add_tuner_bool(                                             \
            ctx,                                                       \
            l.current_x,                                               \
            ui_layout_next_y(&l),                                      \
            l.btn_w,                                                   \
            l.btn_h,                                                   \
            name_str,                                                  \
            PARAM_##id,                                                \
            tooltip);                                                  \
    }                                                                  \
    else                                                               \
    {                                                                  \
        ui_add_tuner_float(                                            \
            ctx,                                                       \
            l.current_x,                                               \
            ui_layout_next_y(&l),                                      \
            l.btn_w,                                                   \
            l.btn_h,                                                   \
            name_str,                                                  \
            PARAM_##id,                                                \
            min_val,                                                   \
            max_val,                                                   \
            tooltip);                                                  \
    }

#define UI_PARAM_TUNER_V3(type, id, name_str, min_val, max_val, tooltip) \
    ui_add_tuner_v3(ctx, l.current_x, ui_layout_next_y(&l), l.btn_w, l.btn_h, name_str, PARAM_##id, min_val, max_val, tooltip, &s_macro_param_registry[PARAM_##id].v3);

#define UI_PARAM_TUNER_V4(type, id, name_str, min_val, max_val, tooltip) \
    ui_add_tuner_v4(ctx, l.current_x, ui_layout_next_y(&l), l.btn_w, l.btn_h, name_str, PARAM_##id, min_val, max_val, tooltip, &s_macro_param_registry[PARAM_##id].v4);

void setup_camera_controls(UI_Context* ctx)
{
    UI_Layout l = ui_layout_begin(ctx);

    #define X UI_PARAM_TUNER
    CAMERA_PARAMS_MAP
    #undef X
    #define X UI_PARAM_TUNER_V3
    CAMERA_PARAMS_V3_MAP
    #undef X
}

void setup_debug_controls(UI_Context* ctx) {
    UI_Layout l = ui_layout_begin(ctx);
    char* txt_src = load_file("../../config/g_debug_log_filter.txt");
    ui_add_scrollable_text_editor(
        ctx,
        l.current_x,
        ui_layout_next_y(&l),
        get_param_float(PARAM_UI_EDITOR_WIDTH),
        get_param_float(PARAM_UI_EDITOR_HEIGHT),
        txt_src,
        "../../config/g_debug_log_filter.txt");
        free(txt_src);
}

void setup_fx_controls(UI_Context* ctx){} //particle systems, params for weather, wind patterns etc

void setup_input_controls(UI_Context* ctx){
    UI_Layout l = ui_layout_begin(ctx);

    #define X UI_PARAM_TUNER
    INPUT_PARAMS_MAP
    #undef X
    #define X UI_PARAM_TUNER_ENUM
    INPUT_BIND_PARAMS_MAP
    #undef X
}
void setup_lights_controls(UI_Context* ctx){}// this will visualize light sources, let you select and move points, attach lights to meshes
// change color, attenutation, intensity
void setup_main_controls(UI_Context* ctx)
{
    LoadState* state = malloc(sizeof(LoadState));
    state->ctx = ctx;
    platform_http_get(
        "https://raw.githubusercontent.com/christinespades/ildzium-engine/main/README.md",
        on_readme_loaded,
        state
    );
}
void setup_meshes_controls(UI_Context* ctx){}// select meshes,, outline, gizmo, transform, vertex edit/scuplt, change material, texture


void setup_render_controls(UI_Context* ctx)
{
    UI_Layout l = ui_layout_begin(ctx);

    #define X UI_PARAM_TUNER
    RENDER_PARAMS_MAP
    #undef X
    #define X UI_PARAM_TUNER_V4
    RENDER_PARAMS_V4_MAP
    #undef X
}

void setup_shaders_controls(UI_Context* ctx){
    UI_Layout l = ui_layout_begin(ctx);

    #define X UI_PARAM_TUNER
    SHADERS_SKY_PARAMS_MAP
    #undef X
    #define X UI_PARAM_TUNER_V3
    SHADERS_SKY_PARAMS_V3_MAP
    #undef X

    char* shader_src = load_file("../../shaders/sky.frag");
    ui_add_scrollable_text_editor(ctx,
        900, 70,
        get_param_float(PARAM_UI_EDITOR_WIDTH), get_param_float(PARAM_UI_EDITOR_HEIGHT),
        shader_src,
        "../../shaders/sky.frag");
    free(shader_src);
}

void setup_sounds_controls(UI_Context* ctx){}// this context will enable visualization for 3d sound sources
// so you get gizmos, you select one and get an outline too
// and the params change and affect the one source you select
// so you can adjust the panning, volume, filepath, etc..
// you also get sound controls for the currently loaded tile/area/map
// so you can have a playlist of songs just for a specific location
// or background ambience
// probabilities/variables triggering music etc, specific rules for areas
void setup_terrain_controls(UI_Context* ctx){}//  this will let you manipulate the terrain in the loaded area/map
// so sculpt, paint, inbuilt height lerps and stuff like that, level of detail/random noise gen for colors
// and for topology/dynomorph stuff
void setup_ui_controls(UI_Context* ctx){
    UI_Layout l = ui_layout_begin(ctx);

    #define X UI_PARAM_TUNER
    UI_PARAMS_MAP
    #undef X
}

void ui_set_mode(UI_Context* ctx, UI_Mode mode) {
	// LOGI("Setting UI mode to %s..", enum_str(ui_mode_to_string, mode, "UI_Mode"));
    ctx->current_mode = mode;

    // Clear ALL buttons + reset held state (prevents stale held data on rebuild)
    ctx->button_count = 0;
    memset(ctx->button_held_last_frame, 0, get_param_float(PARAM_UI_MAX_BUTTONS) * sizeof(uint8_t));

    add_top_navigation_buttons(ctx);

    switch (mode) {
        #define X(name, mode_enum)          \
            case mode_enum:                 \
                setup_##name##_controls(ctx); \
                break;

        UI_MODES_MAP
        #undef X

        default:
            break;
    }
}


void project_on_create_btn_clicked()
{
    project_show_new_dialog();
}

void project_on_load_clicked(UI_Button* b)
{
    if (!b) return;
    
    // Diagnostic Fallback: If tooltip was dropped, try reading content text array directly
    const char* target_name = b->tooltip;
    if (!target_name || target_name[0] == '\0') {
        target_name = b->content;
    }

    if (!target_name || target_name[0] == '\0') {
        LOGE("Both tooltip and content are empty on this button instance!");
        return;
    }

    project_load(target_name);
}

void project_on_rename_btn_clicked(UI_Button* b)
{
    if (!b || !b->tooltip) return;
    // Pull the target project name out of the tooltip string!
    project_show_rename_dialog(b->tooltip);
}

void project_on_delete_clicked(UI_Button* b)
{
    if (!b || !b->tooltip) return;
    // Pull the target project name out of the tooltip string!
    project_delete(b->tooltip);
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
    LOGI("Setting up project controls..");
    int proj_count = 0;
    Project* projects = project_get_list(&proj_count);

    UI_Layout l = ui_layout_begin_projects(ctx, proj_count);

    float current_y = ui_layout_next_y(&l);
    ui_add_button(ctx, l.current_x, current_y, l.btn_w, l.btn_h, 
                  "PROJECT MANAGER", NULL, NULL, NULL, NULL, NULL);
    l.idx++; 

    for (int i = 0; i < proj_count; i++)
    {
        current_y = ui_layout_next_y(&l);

        float load_btn_w   = l.btn_w - 140.0f; 
        float action_btn_w = 65.0f;
        float gap          = 5.0f;

        // 1. Load Button: content is the project name
        ui_add_button(ctx, l.current_x, current_y, load_btn_w, l.btn_h, 
                      projects[i].name, (UI_ButtonCallback)project_on_load_clicked, 
                      NULL, NULL, NULL, (const char*)projects[i].name); // <-- Pass it here!

        // 2. Rename Button: Pass project name as the tooltip parameter
        ui_add_button(ctx, l.current_x + load_btn_w + gap, current_y, action_btn_w, l.btn_h, 
                      "Rename", (UI_ButtonCallback)project_on_rename_btn_clicked, 
                      NULL, NULL, NULL, (const char*)projects[i].name);

        // 3. Delete Button: Pass project name as the tooltip parameter
        ui_add_button(ctx, l.current_x + load_btn_w + action_btn_w + (gap * 2), current_y, action_btn_w, l.btn_h, 
                      "Delete", (UI_ButtonCallback)project_on_delete_clicked, 
                      NULL, NULL, NULL, (const char*)projects[i].name);

        l.idx++;
    }

    current_y = ui_layout_next_y(&l);
    ui_add_button(ctx, l.current_x, current_y, l.btn_w, l.btn_h, 
                  "Create New Project", (UI_ButtonCallback)project_on_create_btn_clicked, 
                  NULL, NULL, NULL, NULL);

    if (g_show_project_modal) {
        draw_project_modal(ctx);
    }
}
