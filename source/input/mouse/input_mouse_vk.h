#pragma once
#include "scene/camera.h"
#include "ui/ui_context.h"
#include "core/params/params.h"
#include "input/input_action.h"

extern int mouse_wheel;
void set_mouse_callbacks();
void platform_get_mouse_pos(double* x, double* y);
int  platform_get_mouse_button(int button);
extern double g_mouse_x;
extern double g_mouse_y;
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);