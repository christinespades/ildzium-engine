#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "device.h"
#include "ui.h"
#include "ui_renderer.h"
#include "math.h"
#include "memory.h"
#include "model.h"
#include "renderer_draw.h"
#include "shaders.h"
#include "sky.h"

extern CameraUBO cameraUBOData;

VkDescriptorSetLayout modelDescriptorSetLayout = VK_NULL_HANDLE;
VkDescriptorPool modelDescriptorPool = VK_NULL_HANDLE;
VkDescriptorSet modelDescriptorSet = VK_NULL_HANDLE;

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

VkPipeline modelPipeline = VK_NULL_HANDLE;
VkPipelineLayout modelPipelineLayout = VK_NULL_HANDLE;
VkShaderModule modelVertModule = VK_NULL_HANDLE;
VkShaderModule modelFragModule = VK_NULL_HANDLE;
VkImage depthImage = VK_NULL_HANDLE;
VkDeviceMemory depthMemory = VK_NULL_HANDLE;
VkImageView depthImageView = VK_NULL_HANDLE;
VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

// ====================== FORWARD DECLARATIONS ======================
void init_renderer(VkInstance instance, VkSurfaceKHR surface);
void cleanup_renderer();
extern void draw_frame();

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

    vkCreateImage(device, &imageInfo, NULL, &depthImage);

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(device, depthImage, &memReq);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(device, &allocInfo, NULL, &depthMemory);
    vkBindImageMemory(device, depthImage, depthMemory, 0);

    VkImageViewCreateInfo viewInfo = {0};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = depthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = depthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(device, &viewInfo, NULL, &depthImageView);
}

// ====================== RENDER PASS (now with depth) ======================
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

    vkCreateRenderPass(device, &rpInfo, NULL, &renderPass);
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

        vkCreateFramebuffer(device, &fbInfo, NULL, &framebuffers[i]);
    }
}

static void create_camera_ubo(void)
{
    VkDeviceSize size = sizeof(CameraUBO);
    create_vulkan_buffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         &cameraUBOBuffer, &cameraUBOMemory);
}

static void create_model_descriptors(void)
{
    VkDescriptorSetLayoutBinding bindings[2] = {0};

    // 0: Camera UBO
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // 1: Instance SSBO
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;
    vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &modelDescriptorSetLayout);

    VkDescriptorPoolSize poolSizes[2] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}
    };
    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 1;
    vkCreateDescriptorPool(device, &poolInfo, NULL, &modelDescriptorPool);

    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = modelDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &modelDescriptorSetLayout;
    vkAllocateDescriptorSets(device, &allocInfo, &modelDescriptorSet);

    // Initial camera UBO write
    VkDescriptorBufferInfo camInfo = { cameraUBOBuffer, 0, sizeof(CameraUBO) };
    VkWriteDescriptorSet write = {0};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = modelDescriptorSet;
    write.dstBinding = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &camInfo;
    vkUpdateDescriptorSets(device, 1, &write, 0, NULL);
}

void update_model_descriptor(void)   // public for model.c
{
    VkDescriptorBufferInfo instInfo = { g_model.instanceBuffer, 0, g_model.instanceBufferSize };
    VkWriteDescriptorSet write = {0};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = modelDescriptorSet;
    write.dstBinding = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &instInfo;
    vkUpdateDescriptorSets(device, 1, &write, 0, NULL);
}

// ====================== INIT RENDERER ======================
void init_renderer(VkInstance instance, VkSurfaceKHR surface)
{
    if (ui_framebuffer) {
        free(ui_framebuffer);           // avoid memory leak if called multiple times
    }

    pick_physical_device(instance, surface);
    create_logical_device();

    // === SWAPCHAIN ===
    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps);

    if (surfaceCaps.currentExtent.width == 0 || surfaceCaps.currentExtent.height == 0) {
        printf("Window has zero size\n");
        exit(1);
    }
    swapchainExtent = surfaceCaps.currentExtent;

    // Surface format
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);
    VkSurfaceFormatKHR* formats = malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats);

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
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
    VkPresentModeKHR* presentModes = malloc(sizeof(VkPresentModeKHR) * presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);

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
    swapCreateInfo.surface = surface;
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

    if (vkCreateSwapchainKHR(device, &swapCreateInfo, NULL, &swapchain) != VK_SUCCESS) {
        printf("Failed to create swapchain\n");
        exit(1);
    }

    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, NULL);
    swapchainImages = malloc(sizeof(VkImage) * swapchainImageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages);

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
        vkCreateImageView(device, &viewInfo, NULL, &swapchainImageViews[i]);
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

    create_depth_resources();           // ← new
    create_render_pass();               // ← updated
    create_framebuffers();              // ← updated (now 2 attachments)

    // Command pool
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // <-- here
    vkCreateCommandPool(device, &poolInfo, NULL, &commandPool);

    // Command buffers
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;  // allocate N frames
    vkAllocateCommandBuffers(device, &allocInfo, commandBuffers);

    VkSemaphoreCreateInfo semInfo = {0};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkCreateSemaphore(device, &semInfo, NULL, &imageAvailable[i]);
        vkCreateSemaphore(device, &semInfo, NULL, &renderFinished[i]);
        vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[i]);
    }

    create_camera_ubo();
    create_model_descriptors();
    sky_init();
    ui_renderer_init();

    init_model_system();
    update_model_descriptor();         // initial empty buffer
    printf("Renderer initialized\n");
}

// ====================== CLEANUP ======================
void cleanup_renderer()
{
    vkDeviceWaitIdle(device);

    cleanup_model_system();
    sky_cleanup();

    vkDestroyBuffer(device, cameraUBOBuffer, NULL);
    vkFreeMemory(device, cameraUBOMemory, NULL);

    vkDestroyDescriptorSetLayout(device, modelDescriptorSetLayout, NULL);
    vkDestroyDescriptorPool(device, modelDescriptorPool, NULL);

    // your original cleanup code
    for (uint32_t i = 0; i < swapchainImageCount; i++) {
        vkDestroyFramebuffer(device, framebuffers[i], NULL);
        vkDestroyImageView(device, swapchainImageViews[i], NULL);
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
        vkDestroySemaphore(device, imageAvailable[i], NULL);
        vkDestroySemaphore(device, renderFinished[i], NULL);
        vkDestroyFence(device, inFlightFences[i], NULL);
    }
    vkDestroyCommandPool(device, commandPool, NULL);
    vkDestroyRenderPass(device, renderPass, NULL);
    vkDestroySwapchainKHR(device, swapchain, NULL);
    vkDestroyDevice(device, NULL);
}