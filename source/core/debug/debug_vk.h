#pragma once

void name_vk_obj(VkDevice device, uint64_t objectHandle, VkObjectType objectType, const char* name);

#define VK_CHECK(x)                                     \
do                                                      \
{                                                       \
    VkResult err = (x);                                 \
    if(err != VK_SUCCESS)                               \
    {                                                   \
        LOGE("Vulkan error %d (%s:%d)",                 \
             err, __FILE__, __LINE__);                  \
        exit(1);                                        \
    }                                                   \
} while(0)