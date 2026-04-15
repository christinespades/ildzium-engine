#ifndef __EMSCRIPTEN__
    #pragma once
    #include "scene/camera.h"
    #include "ui/ui.h"

    void vulkan_init(void);
    void vulkan_glfw_shutdown(void);
    void vulkan_draw(void);
    void create_swapchain();
    void recreate_swapchain();
    extern VkDevice device;
    extern uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    extern void create_vulkan_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                     VkBuffer* buffer, VkDeviceMemory* memory);
    extern void update_model_descriptor(void);
    extern void ui_renderer_init(void);
    extern void ui_renderer_upload(uint32_t* ui_pixels, uint32_t width, uint32_t height);
    extern void ui_renderer_draw(VkCommandBuffer cmd);
    VkExtent2D swapchainExtent;
#endif