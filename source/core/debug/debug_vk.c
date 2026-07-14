#include "pch.h"
#ifndef __EMSCRIPTEN__
    void name_vk_obj(VkDevice device, uint64_t objectHandle, VkObjectType objectType, const char* name) {
        PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT = 
            (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");

        if (pfnSetDebugUtilsObjectNameEXT != NULL) {
            VkDebugUtilsObjectNameInfoEXT nameInfo;
            nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            nameInfo.pNext = NULL;
            nameInfo.objectType = objectType;
            nameInfo.objectHandle = objectHandle;
            nameInfo.pObjectName = name;

            pfnSetDebugUtilsObjectNameEXT(device, &nameInfo);
        }
    }
#endif