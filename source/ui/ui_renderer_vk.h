#pragma once
#include "core/memory.h"
#include "rendering/fps.h"
#include "rendering/renderer.h"
#include "rendering/shaders_vk.h"

void ui_renderer_init(void);
void ui_renderer_upload(void);
void ui_renderer_draw(VkCommandBuffer cmd);
void ui_renderer_resize();