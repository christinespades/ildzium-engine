#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "scene/camera.h"
#include "ui/ui.h"

void init_renderer(VkInstance instance, VkSurfaceKHR surface);
void cleanup_renderer(void);
void draw_frame(void);
extern VkDevice device;
extern uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
extern void create_vulkan_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                 VkBuffer* buffer, VkDeviceMemory* memory);
extern void update_model_descriptor(void);
extern void ui_renderer_init(void);
extern void ui_renderer_upload(uint32_t* ui_pixels, uint32_t width, uint32_t height);
extern void ui_renderer_draw(VkCommandBuffer cmd);
VkExtent2D swapchainExtent;