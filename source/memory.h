#pragma once
#include <vulkan/vulkan.h>

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
void create_vulkan_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                 VkBuffer* buffer, VkDeviceMemory* memory);