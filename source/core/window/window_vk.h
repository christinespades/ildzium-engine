#pragma once
#include "core/window/window.h"

extern GLFWwindow* g_window;
extern VkInstance vk_instance;
extern VkSurfaceKHR vk_surface;
static int framebufferResized;
static void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
int ildz_should_window_close();
void init_window(void);
void ildz_get_window_size(int* w, int* h);