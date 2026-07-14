#pragma once
#include "rendering/renderer.h"

uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);
void create_vulkan_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer* buffer, VkDeviceMemory* memory);
void create_vulkan_buffer_mapped(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer* buffer, VkDeviceMemory* memory, const void* mappedData);