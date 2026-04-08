#pragma once
#include <stdint.h>
#include <vulkan/vulkan.h>
#include "core/memory.h"
#include "rendering/renderer.h"
#include "rendering/shaders.h"

void ui_renderer_init(void);
void ui_renderer_upload(uint32_t* ui_pixels, uint32_t width, uint32_t height);
void ui_renderer_draw(VkCommandBuffer cmd);
void ui_renderer_cleanup(void);

extern uint32_t* ui_framebuffer;