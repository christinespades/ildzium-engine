#include "pch.h"
#ifndef __EMSCRIPTEN__
    #include "core/memory.h"

    extern VkDevice vk_device;
    extern VkPhysicalDevice physicalDevice;

    uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

        for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        LOGE("Failed to find suitable memory type!");
        exit(1);
    }

    void create_vulkan_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer* buffer, VkDeviceMemory* memory)
    {
        VkBufferCreateInfo bufInfo = {0};
        bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufInfo.size = size;
        bufInfo.usage = usage;
        bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK((vkCreateBuffer(vk_device, &bufInfo, NULL, buffer) != VK_SUCCESS));

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(vk_device, *buffer, &memReq);

        VkMemoryAllocateInfo allocInfo = {0};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = find_memory_type(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VK_CHECK((vkAllocateMemory(vk_device, &allocInfo, NULL, memory) != VK_SUCCESS));
        vkBindBufferMemory(vk_device, *buffer, *memory, 0);
    }

    void create_vulkan_buffer_mapped(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer* buffer, VkDeviceMemory* memory, const void* mappedData)
    {
        create_vulkan_buffer(size, usage, buffer, memory);
        void* data;
        VK_CHECK(vkMapMemory(vk_device, *memory, 0, size, 0, &data));
        memcpy(data, mappedData, size);
        vkUnmapMemory(vk_device, *memory);
    }
#endif