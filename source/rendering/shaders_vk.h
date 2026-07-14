#pragma once
#include "rendering/device.h"
#include "rendering/renderer_debug_compute_vk.h"
#include "scene/model_vk.h"
#include "scene/sky_vk.h"
#include "ui/ui.h"

uint32_t* load_spirv(const char* filename, size_t* out_size);
void on_shader_changed(const char* path);
void load_shader_pair(const char* shaderName, VkShaderModule* outVertModule, VkShaderModule* outFragModule);
void load_compute_shader(const char* shaderName, VkShaderModule* outComputeModule);