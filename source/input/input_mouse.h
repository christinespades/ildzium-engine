#pragma once
#include "core/window.h"
#include "scene/camera.h"
#include "ui/ui_context.h"

extern int mouse_wheel;
void set_mouse_callbacks();
void platform_get_mouse_pos(double* x, double* y);
int  platform_get_mouse_button(int button);

#ifndef __EMSCRIPTEN__
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
#else
    extern int g_mouse_buttons[3];
    extern double g_mouse_x;
    extern double g_mouse_y;
#endif