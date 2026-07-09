#include "pch.h"
#include "ui_callbacks.h"

// Generic event whenever any button is held down
void on_button_held(void) {
    //printf("Button HELD!\n");
}

void on_button_hovered(void) {
}

void on_layout_box_clicked(void) {}

#define UI_MODE_LIST \
    X(camera,  UI_MODE_CAMERA)  \
    X(fx,      UI_MODE_FX)      \
    X(input,   UI_MODE_INPUT)   \
    X(lights,  UI_MODE_LIGHTS)  \
    X(main,    UI_MODE_MAIN)    \
    X(meshes,  UI_MODE_MESHES)  \
    X(project, UI_MODE_PROJECT) \
    X(skybox,  UI_MODE_SKYBOX)  \
    X(sounds,  UI_MODE_SOUNDS)  \
    X(terrain, UI_MODE_TERRAIN) \
    X(ui,      UI_MODE_UI)

#define X(name, mode) \
    void on_##name##_clicked(void) { if (g_ui_ctx) ui_set_mode(g_ui_ctx, mode); }
UI_MODE_LIST
#undef X