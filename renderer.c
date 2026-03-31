#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>   // for memset
#include <GLFW/glfw3.h>
#include "camera.h"
#include "ui.h"
#include "ui_renderer.h"
#include "memory.h"
#include "model.h"
#include "shaders.h"

// Push constant for sky (time + camera rotation)
typedef struct {
    float time;
    float yaw;      // in radians
    float pitch;    // in radians
} SkyPushConstant;

typedef struct {
    float view[16];
    float proj[16];
} CameraUBO;

VkBuffer cameraUBOBuffer = VK_NULL_HANDLE;
VkDeviceMemory cameraUBOMemory = VK_NULL_HANDLE;
CameraUBO cameraUBOData;

VkDescriptorSetLayout modelDescriptorSetLayout = VK_NULL_HANDLE;
VkDescriptorPool modelDescriptorPool = VK_NULL_HANDLE;
VkDescriptorSet modelDescriptorSet = VK_NULL_HANDLE;
// ====================== GLOBALS ======================
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device = VK_NULL_HANDLE;
VkQueue graphicsQueue = VK_NULL_HANDLE;
VkSwapchainKHR swapchain = VK_NULL_HANDLE;
VkFormat swapchainFormat;
VkExtent2D swapchainExtent;
VkRenderPass renderPass = VK_NULL_HANDLE;
VkFramebuffer* framebuffers = NULL;
VkImageView* swapchainImageViews = NULL;
VkImage* swapchainImages = NULL;
VkCommandPool commandPool = VK_NULL_HANDLE;
uint32_t swapchainImageCount = 0;
uint32_t queueFamilyIndex = 0;
#define MAX_FRAMES_IN_FLIGHT 2
VkSemaphore imageAvailable[MAX_FRAMES_IN_FLIGHT];
VkSemaphore renderFinished[MAX_FRAMES_IN_FLIGHT];
VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
uint32_t currentFrame = 0;

// New for sky
VkPipeline skyPipeline = VK_NULL_HANDLE;
VkPipelineLayout skyPipelineLayout = VK_NULL_HANDLE;
VkShaderModule vertShaderModule = VK_NULL_HANDLE;
VkShaderModule fragShaderModule = VK_NULL_HANDLE;

VkPipeline modelPipeline = VK_NULL_HANDLE;
VkPipelineLayout modelPipelineLayout = VK_NULL_HANDLE;
VkShaderModule modelVertModule = VK_NULL_HANDLE;
VkShaderModule modelFragModule = VK_NULL_HANDLE;
VkImage depthImage = VK_NULL_HANDLE;
VkDeviceMemory depthMemory = VK_NULL_HANDLE;
VkImageView depthImageView = VK_NULL_HANDLE;
VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

extern void ui_renderer_init(void);
extern void ui_renderer_upload(uint32_t* ui_pixels, uint32_t width, uint32_t height);
extern void ui_renderer_draw(VkCommandBuffer cmd);

// ====================== FORWARD DECLARATIONS ======================
void init_renderer(VkInstance instance, VkSurfaceKHR surface);
void cleanup_renderer();
void draw_frame();

static void create_sky_pipeline();
static void create_depth_resources(void);
static void create_render_pass(void);           // depth + color
static void create_model_pipeline(void);

// Helper to create buffers (kept simple and consistent with your original style)
void create_vulkan_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                 VkBuffer* buffer, VkDeviceMemory* memory)
{
    VkBufferCreateInfo bufInfo = {0};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = size;
    bufInfo.usage = usage;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufInfo, NULL, buffer) != VK_SUCCESS) {
        printf("Failed to create buffer\n");
        exit(1);
    }

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device, *buffer, &memReq);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(device, &allocInfo, NULL, memory) != VK_SUCCESS) {
        printf("Failed to allocate buffer memory\n");
        exit(1);
    }
    vkBindBufferMemory(device, *buffer, *memory, 0);
}

// ====================== PIPELINE CREATION ======================
static void create_sky_pipeline()
{
    // Load shaders (adjust path if needed)
    size_t vert_size, frag_size;
    uint32_t* vert_code = load_spirv("../../shaders/sky.vert.spv", &vert_size);
    uint32_t* frag_code = load_spirv("../../shaders/sky.frag.spv", &frag_size);

    if (!vert_code || !frag_code) {
        printf("Failed to load sky shaders!\n");
        exit(1);
    }

    VkShaderModuleCreateInfo vertCreate = {0};
    vertCreate.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertCreate.codeSize = vert_size;
    vertCreate.pCode = vert_code;
    vkCreateShaderModule(device, &vertCreate, NULL, &vertShaderModule);
    free(vert_code);

    VkShaderModuleCreateInfo fragCreate = {0};
    fragCreate.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragCreate.codeSize = frag_size;
    fragCreate.pCode = frag_code;
    vkCreateShaderModule(device, &fragCreate, NULL, &fragShaderModule);
    free(frag_code);

    // Shader stages
    VkPipelineShaderStageCreateInfo shaderStages[2] = {0};

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertShaderModule;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragShaderModule;
    shaderStages[1].pName = "main";

    // No vertex input (we use gl_VertexIndex)
    VkPipelineVertexInputStateCreateInfo vertexInput = {0};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    // Input assembly - triangle list
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport & scissor (dynamic, but we set full screen)
    VkPipelineViewportStateCreateInfo viewportState = {0};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Multisampling off
    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Depth/stencil - disabled for sky
    VkPipelineDepthStencilStateCreateInfo depthStencil = {0};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;

    // Color blend - simple replace
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending = {0};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Dynamic state (viewport + scissor)
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = {0};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    // Push constant for time (float)
    VkPushConstantRange pushConstant = {0};
    pushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(SkyPushConstant);   // time

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &skyPipelineLayout) != VK_SUCCESS) {
        printf("Failed to create pipeline layout\n");
        exit(1);
    }

    // Create graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = skyPipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &skyPipeline) != VK_SUCCESS) {
        printf("Failed to create sky pipeline\n");
        exit(1);
    }

    printf("Sky pipeline created successfully\n");
}

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

void create_model_pipeline(void)
{
    size_t vert_size, frag_size;
    uint32_t* vert_code = load_spirv("../../shaders/model.vert.spv", &vert_size);
    uint32_t* frag_code = load_spirv("../../shaders/model.frag.spv", &frag_size);
    if (!vert_code || !frag_code) {
        printf("Failed to load model shaders!\n");
        exit(1);
    }

    VkShaderModuleCreateInfo sm = {0};
    sm.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    sm.codeSize = vert_size; sm.pCode = vert_code;
    vkCreateShaderModule(device, &sm, NULL, &modelVertModule);
    free(vert_code);

    sm.codeSize = frag_size; sm.pCode = frag_code;
    vkCreateShaderModule(device, &sm, NULL, &modelFragModule);
    free(frag_code);

    VkPipelineShaderStageCreateInfo stages[2] = {0};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = modelVertModule;
    stages[0].pName = "main";

    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = modelFragModule;
    stages[1].pName = "main";

    // Vertex input
    VkVertexInputBindingDescription binding = {0};
    binding.binding = 0;
    binding.stride = sizeof(Vertex3D);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrs[3] = {0};
    attrs[0].location = 0; attrs[0].binding = 0; attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT; attrs[0].offset = offsetof(Vertex3D, x);
    attrs[1].location = 1; attrs[1].binding = 0; attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT; attrs[1].offset = offsetof(Vertex3D, nx);
    attrs[2].location = 2; attrs[2].binding = 0; attrs[2].format = VK_FORMAT_R32G32_SFLOAT;    attrs[2].offset = offsetof(Vertex3D, u);

    VkPipelineVertexInputStateCreateInfo vertexInput = {0};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &binding;
    vertexInput.vertexAttributeDescriptionCount = 3;
    vertexInput.pVertexAttributeDescriptions = attrs;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState = {0};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo raster = {0};
    raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.lineWidth = 1.0f;
    raster.cullMode = VK_CULL_MODE_NONE;
    raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo ms = {0};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // === DEPTH STENCIL (this was missing from pipeInfo) ===
    VkPipelineDepthStencilStateCreateInfo depthStencil = {0};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState blendAttach = {0};
    blendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                 VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo blend = {0};
    blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend.attachmentCount = 1;
    blend.pAttachments = &blendAttach;

    VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn = {0};
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = 2;
    dyn.pDynamicStates = dynStates;

    // === NEW: descriptor layout instead of push constant ===
    VkPipelineLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &modelDescriptorSetLayout;   // <-- this is the key change

    if (vkCreatePipelineLayout(device, &layoutInfo, NULL, &modelPipelineLayout) != VK_SUCCESS) {
        printf("Failed to create model pipeline layout\n");
        exit(1);
    }

    VkGraphicsPipelineCreateInfo pipeInfo = {0};
    pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeInfo.stageCount = 2;
    pipeInfo.pStages = stages;
    pipeInfo.pVertexInputState = &vertexInput;
    pipeInfo.pInputAssemblyState = &inputAssembly;
    pipeInfo.pViewportState = &viewportState;
    pipeInfo.pRasterizationState = &raster;
    pipeInfo.pMultisampleState = &ms;
    pipeInfo.pDepthStencilState = &depthStencil;        // ← THIS WAS MISSING
    pipeInfo.pColorBlendState = &blend;
    pipeInfo.pDynamicState = &dyn;
    pipeInfo.layout = modelPipelineLayout;
    pipeInfo.renderPass = renderPass;
    pipeInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeInfo, NULL, &modelPipeline) != VK_SUCCESS) {
        printf("Failed to create model pipeline\n");
        exit(1);
    }

    printf("Model pipeline created successfully\n");
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

void update_camera_ubo(void)
{
    float view[16], proj[16];
    camera_get_view_matrix(view);
    int w, h;
    glfwGetWindowSize(g_window, &w, &h);
    camera_get_projection_matrix(proj, (float)w / (float)h);

    memcpy(cameraUBOData.view, view, 16 * sizeof(float));
    memcpy(cameraUBOData.proj, proj, 16 * sizeof(float));

    void* data;
    vkMapMemory(device, cameraUBOMemory, 0, sizeof(CameraUBO), 0, &data);
    memcpy(data, &cameraUBOData, sizeof(CameraUBO));
    vkUnmapMemory(device, cameraUBOMemory);
}

// ====================== INIT RENDERER ======================
void init_renderer(VkInstance instance, VkSurfaceKHR surface)
{
    if (ui_framebuffer) {
        free(ui_framebuffer);           // avoid memory leak if called multiple times
    }

    // Physical device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0) { 
        printf("No GPU with Vulkan support found\n"); 
        exit(1); 
    }

    VkPhysicalDevice devices[16];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
    physicalDevice = devices[0];

    // Queue family
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

    // Logical device
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo deviceCreateInfo = {0};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = 1;
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device) != VK_SUCCESS) {
        printf("Failed to create logical device\n"); 
        exit(1);
    }
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &graphicsQueue);

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
    create_sky_pipeline();
    create_model_pipeline();
    ui_renderer_init();

    init_model_system();
    update_model_descriptor();         // initial empty buffer
    printf("Renderer initialized\n");
}

// ====================== DRAW FRAME ======================
void draw_frame()
{
    // Wait for the current frame to be free
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    // Acquire next image from swapchain
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                                            imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        return; // recreate swapchain here if needed

    // Reset and record command buffer for this frame
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo);

    VkRenderPassBeginInfo rpBegin = {0};
    rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.renderPass = renderPass;
    rpBegin.framebuffer = framebuffers[imageIndex];
    rpBegin.renderArea.extent = swapchainExtent;
    VkClearValue clearValues[2] = {
        {{{0.0f, 0.0f, 0.0f, 1.0f}}},   // color
        {{{1.0f, 0}}}                    // depth = 1.0, stencil = 0
    };
    rpBegin.clearValueCount = 2;
    rpBegin.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, skyPipeline);

    VkViewport viewport = {0};
    viewport.x = 0.0f; viewport.y = 0.0f;
    viewport.width = (float)swapchainExtent.width;
    viewport.height = (float)swapchainExtent.height;
    viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    VkRect2D scissor = {{0,0}, swapchainExtent};
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

    // Push constants: time + camera yaw/pitch
    SkyPushConstant push = {0};
    static float time = 0.0f;
    time += 0.016f;

    push.time  = time;
    push.yaw   = camera.yaw   * 0.0174532925f;   // degrees to radians
    push.pitch = camera.pitch * 0.0174532925f;

    vkCmdPushConstants(commandBuffers[currentFrame], 
                       skyPipelineLayout, 
                       VK_SHADER_STAGE_FRAGMENT_BIT, 
                       0, sizeof(SkyPushConstant), &push);

    vkCmdDraw(commandBuffers[currentFrame], 4, 1, 0, 0);
    
    update_camera_ubo();

    // 2. 3D Models
    if (g_model.meshCount > 0) {
        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);

        draw_models(commandBuffers[currentFrame]);
    }

    if (ui_ctx.cursor_captured) {
        ui_draw(&ui_ctx, ui_framebuffer, swapchainExtent.width, swapchainExtent.height);
        ui_renderer_upload(ui_framebuffer, swapchainExtent.width, swapchainExtent.height);
        ui_renderer_draw(commandBuffers[currentFrame]);
    }

    vkCmdEndRenderPass(commandBuffers[currentFrame]);
    vkEndCommandBuffer(commandBuffers[currentFrame]);

    // Submit command buffer
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submit = {0};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &imageAvailable[currentFrame];
    submit.pWaitDstStageMask = waitStages;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &commandBuffers[currentFrame];
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &renderFinished[currentFrame];

    vkQueueSubmit(graphicsQueue, 1, &submit, inFlightFences[currentFrame]);

    // Present frame
    VkPresentInfoKHR present = {0};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &renderFinished[currentFrame];
    present.swapchainCount = 1;
    present.pSwapchains = &swapchain;
    present.pImageIndices = &imageIndex;

    vkQueuePresentKHR(graphicsQueue, &present);

    // Advance to next frame
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// ====================== CLEANUP ======================
void cleanup_renderer()
{
    vkDeviceWaitIdle(device);

    cleanup_model_system();

    vkDestroyBuffer(device, cameraUBOBuffer, NULL);
    vkFreeMemory(device, cameraUBOMemory, NULL);

    vkDestroyDescriptorSetLayout(device, modelDescriptorSetLayout, NULL);
    vkDestroyDescriptorPool(device, modelDescriptorPool, NULL);

    vkDestroyPipeline(device, skyPipeline, NULL);
    vkDestroyPipelineLayout(device, skyPipelineLayout, NULL);
    vkDestroyShaderModule(device, vertShaderModule, NULL);
    vkDestroyShaderModule(device, fragShaderModule, NULL);

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