#pragma once
#include "input/input.h"
#include "rendering/surface.h"
#include "rendering/renderer.h"
#include "ui/ui.h"

extern int g_width;
extern int g_height;
    
#ifndef __EMSCRIPTEN__
    extern GLFWwindow* g_window;
    extern VkInstance vk_instance;
    extern VkSurfaceKHR vk_surface;
    static int framebufferResized;
#endif

#ifdef __EMSCRIPTEN__
    static EM_BOOL on_resize(int eventType, const EmscriptenUiEvent* e, void* userData);
#else
    static void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
#endif

void init_window(void);
#ifndef __EMSCRIPTEN__
    int ildz_should_close();
#endif

void ildz_get_window_size(int* w, int* h);