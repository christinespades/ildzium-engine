#pragma once
#include "core/project.h"
#include "rendering/shaders_vk.h"
#include "rendering/renderer_vk.h"
#include "scene/lights_vk.h"
#include "scene/model_instance_vk.h"
#include "model_vk_load.h"
#include "model_state_generate.h"

VkPipeline modelPipeline;
VkPipelineLayout modelPipelineLayout;
VkDescriptorSet modelDescriptorSet;
VkDescriptorSetLayout modelDescriptorSetLayout;
VkDescriptorPool modelDescriptorPool;

void create_model_descriptors(void);
void update_model_descriptor(void);
void init_model_system(void);
void destroy_model(Model* model);
void cleanup_model_system(void);