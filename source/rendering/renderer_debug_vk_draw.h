#pragma once
#include "renderer_debug_vk.h"
#include "scene/camera.h"

void renderer_debug_draw(VkCommandBuffer cmdBuffer, uint32_t currentFrame, VkExtent2D swapchainExtent);