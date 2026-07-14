#pragma once

#include "rendering/shaders_vk.h"
#include "scene/sky.h"

extern VkPipeline skyPipeline;
extern VkPipelineLayout skyPipelineLayout;
extern VkShaderModule vertShaderModule;
extern VkShaderModule fragShaderModule;

void sky_init(void);
void sky_cleanup(void);
void sky_draw(VkCommandBuffer cmd);