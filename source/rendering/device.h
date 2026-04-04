#pragma once
#include <vulkan/vulkan.h>

extern VkPhysicalDevice physicalDevice;
extern VkDevice device;
extern VkQueue graphicsQueue;
extern uint32_t queueFamilyIndex;

void pick_physical_device(VkInstance instance, VkSurfaceKHR surface);
void create_logical_device();