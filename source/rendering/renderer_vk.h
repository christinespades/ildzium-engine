#pragma once
#include "core/params/params.h"
#include "rendering/renderer_debug_vk.h"
#include "scene/camera.h"
#include "ui/ui.h"
#include "rendering/device.h"
#include "ui/ui_renderer_vk.h"
#include "core/memory.h"
#include "rendering/renderer_vk_draw.h"
#include "rendering/shaders_vk.h"
#include "rendering/surface.h"
#include "scene/lights_vk.h"
#include "scene/model_vk.h"
#include "scene/sky_vk.h"

void vulkan_init(void);
void vulkan_glfw_shutdown(void);
void vulkan_draw(void);
void create_swapchain();
void recreate_swapchain();
extern uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);
extern void create_vulkan_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                 VkBuffer* buffer, VkDeviceMemory* memory);
extern void update_model_descriptor(void);
extern void ui_renderer_init(void);
extern void ui_renderer_upload(void);
extern void ui_renderer_draw(VkCommandBuffer cmd);
VkExtent2D swapchainExtent;