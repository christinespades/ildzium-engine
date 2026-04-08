#include "pch.h"
#include "ui_callbacks.h"

// Generic event whenever any button is held down
void on_button_held(void) {
    //printf("Button HELD!\n");
}

void on_camera_clicked(void) {
    if (g_ui_ctx) ui_set_mode(g_ui_ctx, UI_MODE_CAMERA);
}

void on_fx_clicked(void) {
    if (g_ui_ctx) ui_set_mode(g_ui_ctx, UI_MODE_FX);
}

void on_input_clicked(void) {
    if (g_ui_ctx) ui_set_mode(g_ui_ctx, UI_MODE_INPUT);
}

void on_lights_clicked(void) {
    if (g_ui_ctx) ui_set_mode(g_ui_ctx, UI_MODE_LIGHTS);
}

void on_main_clicked(void) {
    if (g_ui_ctx) ui_set_mode(g_ui_ctx, UI_MODE_MAIN);
}

void on_meshes_clicked(void) {
    if (g_ui_ctx) ui_set_mode(g_ui_ctx, UI_MODE_MESHES);
}

void on_skybox_clicked(void) {
    if (g_ui_ctx) ui_set_mode(g_ui_ctx, UI_MODE_SKYBOX);
}

void on_sounds_clicked(void) {
    if (g_ui_ctx) ui_set_mode(g_ui_ctx, UI_MODE_SOUNDS);
}

void on_terrain_clicked(void) {
    if (g_ui_ctx) ui_set_mode(g_ui_ctx, UI_MODE_TERRAIN);
}