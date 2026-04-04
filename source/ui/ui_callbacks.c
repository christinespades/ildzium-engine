#include "ui_callbacks.h"

// Generic event whenever any button is held down
void on_button_held(void) {
    //printf("Button HELD!\n");
}

void on_main_menu_clicked(void) {
    if (g_ui_ctx) ui_set_mode(g_ui_ctx, UI_MODE_MAIN_MENU);
}

void on_skybox_clicked(void) {
    if (g_ui_ctx) ui_set_mode(g_ui_ctx, UI_MODE_SKYBOX);
}

void on_terrain_clicked(void) {
    if (g_ui_ctx) ui_set_mode(g_ui_ctx, UI_MODE_TERRAIN);
}