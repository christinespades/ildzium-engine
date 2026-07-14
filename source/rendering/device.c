#include "pch.h"
#include "device.h"

#ifdef __EMSCRIPTEN__
    WGPUQueue queue = NULL;
#else
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice vk_device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    uint32_t queueFamilyIndex = 0;
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    VkSampleCountFlagBits get_max_msaa_samples()
    {
        VkPhysicalDeviceProperties properties;

        vkGetPhysicalDeviceProperties(
            physicalDevice,
            &properties
        );


        VkSampleCountFlags counts =
            properties.limits.framebufferColorSampleCounts &
            properties.limits.framebufferDepthSampleCounts;


        if (counts & VK_SAMPLE_COUNT_8_BIT)
            return VK_SAMPLE_COUNT_8_BIT;

        if (counts & VK_SAMPLE_COUNT_4_BIT)
            return VK_SAMPLE_COUNT_4_BIT;

        if (counts & VK_SAMPLE_COUNT_2_BIT)
            return VK_SAMPLE_COUNT_2_BIT;

        return VK_SAMPLE_COUNT_1_BIT;
    }

    void pick_physical_device(VkInstance instance, VkSurfaceKHR surface)
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
        if (deviceCount == 0) {
            LOGE("No GPU with Vulkan support found");
            exit(1);
        }

        VkPhysicalDevice devices[16];
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
        physicalDevice = devices[0];

        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, NULL);
        VkQueueFamilyProperties* props = malloc(sizeof(VkQueueFamilyProperties) * count);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, props);

        queueFamilyIndex = 0;
        for (uint32_t i = 0; i < count; i++) {
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            if ((props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport) {
                queueFamilyIndex = i;
                break;
            }
        }
        free(props);
    }

    void setup_msaa()
    {
        msaaSamples = get_max_msaa_samples();

        switch(msaaSamples)
        {
            case VK_SAMPLE_COUNT_8_BIT:
                //LOGI("MSAA: 8x");
                break;

            case VK_SAMPLE_COUNT_4_BIT:
                //LOGI("MSAA: 4x");
                break;

            case VK_SAMPLE_COUNT_2_BIT:
                //LOGI("MSAA: 2x");
                break;

            default:
                //LOGI("MSAA: disabled");
                break;
        }
    }

    // Create logical device with timeline semaphore feature enabled safely
    void create_logical_device()
    {
        // Query timeline semaphore support
        VkPhysicalDeviceTimelineSemaphoreFeatures timelineFeatures = {};
        timelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
        timelineFeatures.pNext = NULL;
        timelineFeatures.timelineSemaphore = VK_TRUE;

        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo = {0};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        const char* deviceExtensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
        };

        VkDeviceCreateInfo deviceCreateInfo = {0};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = &timelineFeatures;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.enabledExtensionCount = 2;
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;

        if (vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &vk_device) != VK_SUCCESS) {
            LOGE("Failed to create logical device");
            exit(1);
        }

        vkGetDeviceQueue(vk_device, queueFamilyIndex, 0, &graphicsQueue);
    }
#endif