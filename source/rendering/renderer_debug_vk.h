#pragma once
#include "core/memory.h"
#include "core/params/params.h"
#include "rendering/device.h"	
#include "rendering/renderer_debug_add.h"
#include "rendering/renderer_vk_draw.h"
#include "rendering/shaders_vk.h"

void renderer_debug_init();
void renderer_debug_draw(VkCommandBuffer cmdBuffer, uint32_t currentFrame, VkExtent2D swapchainExtent);
void create_debug_pipelines();
void renderer_debug_destroy();