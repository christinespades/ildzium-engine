#include "pch.h"
#include "renderer.h"

#ifdef __EMSCRIPTEN__
    void webgpu_init();
    void webgpu_draw();
    void webgpu_shutdown();
#else
    void vulkan_init();
    void vulkan_draw();
    void vulkan_shutdown();
#endif

void init_renderer()
{
#ifdef __EMSCRIPTEN__
    webgpu_init();
#else
    vulkan_init();
#endif
}

void draw_frame()
{
#ifdef __EMSCRIPTEN__
    webgpu_draw();
#else
    vulkan_draw();
#endif
}

void cleanup_renderer()
{
#ifdef __EMSCRIPTEN__
    webgpu_shutdown();
#else
    vulkan_shutdown();
#endif
}