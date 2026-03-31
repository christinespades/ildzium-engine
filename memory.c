#include <stdio.h>    // FILE, fopen, fread, fseek, ftell, fclose, printf
#include <stdlib.h>   // malloc, free, exit, NULL
#include <stdint.h>   // uint32_t
#include <vulkan/vulkan.h>
#include "renderer.h"

extern VkPhysicalDevice physicalDevice;

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    printf("Failed to find suitable memory type!\n");
    exit(1);
}