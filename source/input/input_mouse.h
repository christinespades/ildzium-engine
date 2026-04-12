#pragma once
#include "core/window.h"

int mouse_wheel;
void set_mouse_callbacks();
void platform_get_mouse_pos(double* x, double* y);
int  platform_get_mouse_button(int button);

#ifndef __EMSCRIPTEN__
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
#endif