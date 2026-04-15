#include "pch.h"
#ifndef __EMSCRIPTEN__
    #include "rendering/renderer.h"
    #include "rendering/device.h"
    #include "ui/ui_renderer.h"
    #include "core/math.h"
    #include "core/memory.h"
    #include "rendering/renderer_vulkan_draw.h"
    #include "rendering/shaders.h"
    #include "rendering/surface.h"
    #include "scene/camera.h"
    #include "scene/lights.h"
    #include "scene/model.h"
    #include "scene/sky.h"

    extern CameraUBO cameraUBOData;
    extern VkDevice vk_device;
    extern VkInstance vk_instance;
    extern VkSurfaceKHR vk_surface;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkExtent2D swapchainExtent;
    VkFormat swapchainFormat;
    extern VkRenderPass renderPass;
    VkFramebuffer* framebuffers = NULL;
    VkImageView* swapchainImageViews = NULL;
    VkImage* swapchainImages = NULL;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    uint32_t swapchainImageCount = 0;
    extern VkSemaphore imageAvailable[MAX_FRAMES_IN_FLIGHT];
    extern VkSemaphore renderFinished[MAX_FRAMES_IN_FLIGHT];
    extern VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
    extern VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];

    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

    void vulkan_init();
    void vulkan_glfw_shutdown();
    extern void vulkan_draw();

    static void create_depth_resources(void);
    static void create_render_pass(void);           // depth + color

    static void create_depth_resources(void)
    {
        VkImageCreateInfo imageInfo = {0};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapchainExtent.width;
        imageInfo.extent.height = swapchainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vkCreateImage(vk_device, &imageInfo, NULL, &depthImage);

        VkMemoryRequirements memReq;
        vkGetImageMemoryRequirements(vk_device, depthImage, &memReq);

        VkMemoryAllocateInfo allocInfo = {0};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkAllocateMemory(vk_device, &allocInfo, NULL, &depthMemory);
        vkBindImageMemory(vk_device, depthImage, depthMemory, 0);

        VkImageViewCreateInfo viewInfo = {0};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = depthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(vk_device, &viewInfo, NULL, &depthImageView);
    }

    static void create_render_pass(void)
    {
        VkAttachmentDescription attachments[2] = {0};

        // Color attachment
        attachments[0].format = swapchainFormat;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // Depth attachment
        attachments[1].format = depthFormat;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorRef = {0};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthRef = {0};
        depthRef.attachment = 1;
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {0};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;
        subpass.pDepthStencilAttachment = &depthRef;

        VkRenderPassCreateInfo rpInfo = {0};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rpInfo.attachmentCount = 2;
        rpInfo.pAttachments = attachments;
        rpInfo.subpassCount = 1;
        rpInfo.pSubpasses = &subpass;

        vkCreateRenderPass(vk_device, &rpInfo, NULL, &renderPass);
    }

    // ====================== FRAMEBUFFERS (now include depth) ======================
    static void create_framebuffers(void)
    {
        framebuffers = malloc(sizeof(VkFramebuffer) * swapchainImageCount);
        for (uint32_t i = 0; i < swapchainImageCount; i++) {
            VkImageView attachments[2] = {
                swapchainImageViews[i],
                depthImageView
            };

            VkFramebufferCreateInfo fbInfo = {0};
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.renderPass = renderPass;
            fbInfo.attachmentCount = 2;
            fbInfo.pAttachments = attachments;
            fbInfo.width = swapchainExtent.width;
            fbInfo.height = swapchainExtent.height;
            fbInfo.layers = 1;

            vkCreateFramebuffer(vk_device, &fbInfo, NULL, &framebuffers[i]);
        }
    }

    static void create_camera_ubo(void)
    {
        VkDeviceSize size = sizeof(CameraUBO);
        create_vulkan_buffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                             &cameraUBOBuffer, &cameraUBOMemory);
    }

    void create_swapchain() {
        VkSurfaceCapabilitiesKHR surfaceCaps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vk_surface, &surfaceCaps);

        if (surfaceCaps.currentExtent.width == 0 || surfaceCaps.currentExtent.height == 0) {
            printf("Window has zero size\n");
            exit(1);
        }
        swapchainExtent = surfaceCaps.currentExtent;

        // Surface format
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vk_surface, &formatCount, NULL);
        VkSurfaceFormatKHR* formats = malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vk_surface, &formatCount, formats);

        swapchainFormat = formats[0].format;
        VkColorSpaceKHR colorSpace = formats[0].colorSpace;
        for (uint32_t i = 0; i < formatCount; i++) {
            if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
                formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                swapchainFormat = formats[i].format;
                colorSpace = formats[i].colorSpace;
                break;
            }
        }
        free(formats);

        // Present mode
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vk_surface, &presentModeCount, NULL);
        VkPresentModeKHR* presentModes = malloc(sizeof(VkPresentModeKHR) * presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vk_surface, &presentModeCount, presentModes);

        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        for (uint32_t i = 0; i < presentModeCount; i++) {
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
        }
        free(presentModes);

        VkSwapchainCreateInfoKHR swapCreateInfo = {0};
        swapCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapCreateInfo.surface = vk_surface;
        swapCreateInfo.minImageCount = surfaceCaps.minImageCount + 1;
        if (swapCreateInfo.minImageCount > surfaceCaps.maxImageCount && surfaceCaps.maxImageCount > 0)
            swapCreateInfo.minImageCount = surfaceCaps.maxImageCount;

        swapCreateInfo.imageFormat = swapchainFormat;
        swapCreateInfo.imageColorSpace = colorSpace;
        swapCreateInfo.imageExtent = swapchainExtent;
        swapCreateInfo.imageArrayLayers = 1;
        swapCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapCreateInfo.preTransform = surfaceCaps.currentTransform;
        swapCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapCreateInfo.presentMode = presentMode;
        swapCreateInfo.clipped = VK_TRUE;
        swapCreateInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(vk_device, &swapCreateInfo, NULL, &swapchain) != VK_SUCCESS) {
            printf("Failed to create swapchain\n");
            exit(1);
        }

        vkGetSwapchainImagesKHR(vk_device, swapchain, &swapchainImageCount, NULL);
        swapchainImages = malloc(sizeof(VkImage) * swapchainImageCount);
        vkGetSwapchainImagesKHR(vk_device, swapchain, &swapchainImageCount, swapchainImages);

        // Image views
        swapchainImageViews = malloc(sizeof(VkImageView) * swapchainImageCount);
        for (uint32_t i = 0; i < swapchainImageCount; i++) {
            VkImageViewCreateInfo viewInfo = {0};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = swapchainImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = swapchainFormat;
            viewInfo.components.r = viewInfo.components.g = 
            viewInfo.components.b = viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;
            vkCreateImageView(vk_device, &viewInfo, NULL, &swapchainImageViews[i]);
        }

        ui_framebuffer = (uint32_t*)malloc(
            (size_t)swapchainExtent.width * swapchainExtent.height * sizeof(uint32_t)
        );

        if (ui_framebuffer == NULL) {
            printf("Failed to allocate UI framebuffer!\n");
            exit(1);   // or handle error properly
        }

        // Zero the buffer (clear to transparent black)
        memset(ui_framebuffer, 0, 
               (size_t)swapchainExtent.width * swapchainExtent.height * sizeof(uint32_t));
    }

    void cleanup_swapchain()
    {
        if (ui_framebuffer) {
            free(ui_framebuffer);
            ui_framebuffer = NULL;
        }

        // Framebuffers
        if (framebuffers) {
            for (uint32_t i = 0; i < swapchainImageCount; i++) {
                vkDestroyFramebuffer(vk_device, framebuffers[i], NULL);
            }
            free(framebuffers);
            framebuffers = NULL;
        }

        // Depth
        if (depthImageView) vkDestroyImageView(vk_device, depthImageView, NULL);
        if (depthImage)     vkDestroyImage(vk_device, depthImage, NULL);
        if (depthMemory)    vkFreeMemory(vk_device, depthMemory, NULL);

        depthImageView = VK_NULL_HANDLE;
        depthImage = VK_NULL_HANDLE;
        depthMemory = VK_NULL_HANDLE;

        // Swapchain image views
        if (swapchainImageViews) {
            for (uint32_t i = 0; i < swapchainImageCount; i++) {
                vkDestroyImageView(vk_device, swapchainImageViews[i], NULL);
            }
            free(swapchainImageViews);
            swapchainImageViews = NULL;
        }

        // Swapchain images array (just free, not destroy)
        if (swapchainImages) {
            free(swapchainImages);
            swapchainImages = NULL;
        }

        // Swapchain
        if (swapchain) {
            vkDestroySwapchainKHR(vk_device, swapchain, NULL);
            swapchain = VK_NULL_HANDLE;
        }
    }

    void recreate_swapchain()
    {
        int width = 0, height = 0;

        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(g_window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(vk_device);

        cleanup_swapchain();

        create_swapchain();
        create_depth_resources();
        create_framebuffers();

        ui_renderer_resize(swapchainExtent.width, swapchainExtent.height);
    }

    void vulkan_init()
    {
        if (ui_framebuffer) {
            free(ui_framebuffer);           // avoid memory leak if called multiple times
        }
        pick_physical_device(vk_instance, vk_surface);
        create_logical_device();
        create_swapchain();
        create_depth_resources();           // ← new
        create_render_pass();               // ← updated
        create_framebuffers();              // ← updated (now 2 attachments)

        // Command pool
        VkCommandPoolCreateInfo poolInfo = {0};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndex;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // <-- here
        vkCreateCommandPool(vk_device, &poolInfo, NULL, &commandPool);

        // Command buffers
        VkCommandBufferAllocateInfo allocInfo = {0};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;  // allocate N frames
        vkAllocateCommandBuffers(vk_device, &allocInfo, commandBuffers);

        VkSemaphoreCreateInfo semInfo = {0};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo = {0};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkCreateSemaphore(vk_device, &semInfo, NULL, &imageAvailable[i]);
            vkCreateSemaphore(vk_device, &semInfo, NULL, &renderFinished[i]);
            vkCreateFence(vk_device, &fenceInfo, NULL, &inFlightFences[i]);
        }

        create_camera_ubo();
        sky_init();
        ui_renderer_init();

        init_lights(vk_device, physicalDevice);
        init_model_system();
        init_lights_write(vk_device);
        printf("Renderer initialized\n");
    }

    void vulkan_glfw_shutdown()
    {
        vkDeviceWaitIdle(vk_device);

        cleanup_model_system();
        sky_cleanup();

        vkDestroyBuffer(vk_device, cameraUBOBuffer, NULL);
        vkFreeMemory(vk_device, cameraUBOMemory, NULL);

        vkDestroyDescriptorSetLayout(vk_device, modelDescriptorSetLayout, NULL);
        vkDestroyDescriptorPool(vk_device, modelDescriptorPool, NULL);

        for (uint32_t i = 0; i < swapchainImageCount; i++) {
            vkDestroyFramebuffer(vk_device, framebuffers[i], NULL);
            vkDestroyImageView(vk_device, swapchainImageViews[i], NULL);
        }
        free(framebuffers);
        free(swapchainImageViews);
        free(swapchainImages);
        free(commandBuffers);
        if (ui_framebuffer) {
            free(ui_framebuffer);
            ui_framebuffer = NULL;
        }

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(vk_device, imageAvailable[i], NULL);
            vkDestroySemaphore(vk_device, renderFinished[i], NULL);
            vkDestroyFence(vk_device, inFlightFences[i], NULL);
        }
        vkDestroyCommandPool(vk_device, commandPool, NULL);
        vkDestroyRenderPass(vk_device, renderPass, NULL);
        vkDestroySwapchainKHR(vk_device, swapchain, NULL);
        vkDestroyDevice(vk_device, NULL);
        vkDestroySurfaceKHR(vk_instance, vk_surface, NULL);
        vkDestroyInstance(vk_instance, NULL);
        glfwDestroyWindow(g_window);
        glfwTerminate();
    }
#endif