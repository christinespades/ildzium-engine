#pragma once
#include "ui.h"
#include "ui/ui_mode.h"

typedef void (*UI_ButtonCallback)(void);
void ui_dispatch_callbacks(UI_Button* b, int is_active, int mouse_pressed, int mouse_pressed_this_frame, int mouse_released_this_frame);
void on_button_held(void);
void on_camera_clicked(void);
void on_debug_clicked(void);
void on_fx_clicked(void);
void on_input_clicked(void);
void on_layout_box_clicked(void);
void on_lights_clicked(void);
void on_main_clicked(void);
void on_meshes_clicked(void);
void on_project_clicked(void);
void on_render_clicked(void);
void on_shaders_clicked(void);
void on_sounds_clicked(void);
void on_terrain_clicked(void);
void on_ui_clicked(void);