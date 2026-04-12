#pragma once

typedef enum {
    RENDERER_BACKEND_VULKAN,
    RENDERER_BACKEND_WEBGPU
} RendererBackend;

#ifdef __EMSCRIPTEN__
    void webgpu_init();
    void webgpu_draw();
    void webgpu_shutdown();
#else
    void vulkan_init();
    void vulkan_draw();
    void vulkan_shutdown();
#endif