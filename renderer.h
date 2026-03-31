#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "ui.h"

void init_renderer(VkInstance instance, VkSurfaceKHR surface);
void cleanup_renderer(void);
void draw_frame(void);

// For models later
void load_model(const char* glb_path);   // we'll implement this soon
extern VkDevice device;
extern uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
extern VkPipeline modelPipeline;
extern VkPipelineLayout modelPipelineLayout;
extern VkDescriptorSet modelDescriptorSet;
extern void update_model_descriptor(void);