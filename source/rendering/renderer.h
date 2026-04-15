#pragma once

typedef enum {
    RENDERER_BACKEND_VULKAN,
    RENDERER_BACKEND_WEBGPU
} RendererBackend;

#ifdef __EMSCRIPTEN__
    void webgpu_init();
    void webgpu_draw(float dt);
    void webgpu_shutdown();
#else
    void vulkan_init();
    void vulkan_draw();
    void vulkan_glfw_shutdown();
#endif