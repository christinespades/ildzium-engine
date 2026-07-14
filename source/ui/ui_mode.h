#pragma once
#include "core/io.h"
#include "core/load_state.h"
#include "scene/sky.h"
#include "ui/ui_callbacks_on_loaded.h"

typedef enum {
    UI_MODE_CAMERA = 0,
    UI_MODE_DEBUG,
    UI_MODE_FX,
    UI_MODE_INPUT,
    UI_MODE_LIGHTS,
    UI_MODE_MAIN,
    UI_MODE_MESHES,
    UI_MODE_PROJECT,
    UI_MODE_RENDER,
    UI_MODE_SHADERS,
    UI_MODE_SOUNDS,
    UI_MODE_TERRAIN,
    UI_MODE_UI,
    UI_MODE_COUNT
} UI_Mode;

#define UI_MODES_MAP \
    X(camera,  UI_MODE_CAMERA)  \
    X(debug,   UI_MODE_DEBUG)  \
    X(fx,      UI_MODE_FX)      \
    X(input,   UI_MODE_INPUT)   \
    X(lights,  UI_MODE_LIGHTS)  \
    X(main,    UI_MODE_MAIN)    \
    X(meshes,  UI_MODE_MESHES)  \
    X(project, UI_MODE_PROJECT) \
    X(render, UI_MODE_RENDER) \
    X(shaders,  UI_MODE_SHADERS)  \
    X(sounds,  UI_MODE_SOUNDS)  \
    X(terrain, UI_MODE_TERRAIN) \
    X(ui,      UI_MODE_UI)

#define X(name, mode_enum) case mode_enum: return #mode_enum;
DEFINE_ENUM_TO_STRING_FUNC(ui_mode_to_string, UI_MODES_MAP)
#undef X

typedef struct UI_Context UI_Context;

void ui_set_mode(UI_Context* ctx, UI_Mode mode);

#include "ui/ui.h"
#include "ui/ui_text.h"
#include "ui/ui_layout.h"
#include "ui/ui_draw_rect_panel.h"
typedef struct UI_Context UI_Context;
void setup_camera_controls(UI_Context* ctx);
void setup_debug_controls(UI_Context* ctx);
void setup_fx_controls(UI_Context* ctx);
void setup_input_controls(UI_Context* ctx);
void setup_lights_controls(UI_Context* ctx);
void setup_main_controls(UI_Context* ctx);
void setup_meshes_controls(UI_Context* ctx);
void setup_project_controls(UI_Context* ctx);
void setup_render_controls(UI_Context* ctx);
void setup_shaders_controls(UI_Context* ctx);

void setup_sounds_controls(UI_Context* ctx);
void setup_terrain_controls(UI_Context* ctx);
void setup_ui_controls(UI_Context* ctx);
void on_skybox_settings_button_clicked(void);
void on_skybox_settings_button_released(void);

#include "core/project.h"

void project_on_load_clicked(UI_Button* b);
void project_on_delete_clicked(UI_Button* b);
void project_on_create_btn_clicked();
void project_on_rename_btn_clicked(UI_Button* b);
void project_modal_on_ok();
void project_modal_on_cancel();