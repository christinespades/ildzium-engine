#include "pch.h"
#ifndef __EMSCRIPTEN__
    #include "rendering/surface.h"

    VkInstance vk_instance = VK_NULL_HANDLE;
    VkSurfaceKHR vk_surface = VK_NULL_HANDLE;

    void create_vk_instance(void)
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        const char* layers[] = { "VK_LAYER_KHRONOS_validation" };

        VkApplicationInfo appInfo = {0};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Ildzium Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Ildzium Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = layers;

        if (vkCreateInstance(&createInfo, NULL, &vk_instance) != VK_SUCCESS) {
            printf("Failed to create Vulkan instance\n");
            exit(1);
        }
    }

    void create_vk_surface(void)
    {
        if (glfwCreateWindowSurface(vk_instance, g_window, NULL, &vk_surface) != VK_SUCCESS) {
            printf("Failed to create window surface\n");
            exit(1);
        }
    }
#endif