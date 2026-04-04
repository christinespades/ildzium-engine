#include "sky.h"
#include "camera.h"
#include "shaders.h"   // assumes load_spirv is here
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <math.h>

SkyParameters g_skyParams = {
    .timeOfDay = 0.3f,          // start a bit after sunrise
    .cycleSpeed = 0.05f,        // slow cycle (adjust to taste, 0 = disabled)

    .nebulaSpeed = 1.0f,
    .nebulaScale = 2.8f,
    .nebulaIntensity = 0.6f,
    .nebulaLayerCount = 4.0f,

    .nebulaColor1 = {0.6f, 0.3f, 0.9f},
    .nebulaColor2 = {0.2f, 0.7f, 1.0f},

    .starCount = 100.0f,
    .starBrightness = 1.1f,
    .starTwinkleSpeed = 6.0f,
    .starSize = 0.12f,

    .auroraIntensity = 0.11f,
    .auroraSpeed = 1.1f,
    .auroraColor = {0.35f, 0.9f, 0.75f},

    .dayNightBlend = 0.0f,
    .vignetteStrength = 0.4f,
    .overallBrightness = 1.0f
};

static VkBuffer skyUBOBuffer = VK_NULL_HANDLE;
static VkDeviceMemory skyUBOMemory = VK_NULL_HANDLE;
static SkyUBO skyUBOData = {0};

static VkDescriptorSetLayout skyDescriptorSetLayout = VK_NULL_HANDLE;
static VkDescriptorPool skyDescriptorPool = VK_NULL_HANDLE;
static VkDescriptorSet skyDescriptorSet = VK_NULL_HANDLE;

// Forward declarations of helpers from renderer.c (we'll keep them there or expose if needed)
extern void create_vulkan_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                 VkBuffer* buffer, VkDeviceMemory* memory);
extern VkDevice device;
extern VkExtent2D swapchainExtent;
extern VkRenderPass renderPass;
extern CameraUBO cameraUBOData;

VkPipeline skyPipeline = VK_NULL_HANDLE;
VkPipelineLayout skyPipelineLayout = VK_NULL_HANDLE;
VkShaderModule vertShaderModule = VK_NULL_HANDLE;
VkShaderModule fragShaderModule = VK_NULL_HANDLE;

void setup_sky_tuners(void)
{
    float btn_w = 380;
    float btn_h = 70;        // slightly taller so value fits nicely
    float start_x = 250;
    float start_y = 100;
    float spacing = 80;      // increased a bit for better readability
    int idx = 0;

    ui_add_tuner(&ui_ctx, start_x + 400, start_y + 4 * spacing, btn_w, btn_h,
                 "Aurora R", &g_skyParams.auroraColor[0], 0.0f, 1.0f);

    ui_add_tuner(&ui_ctx, start_x + 400, start_y + 5 * spacing, btn_w, btn_h,
                 "Aurora G", &g_skyParams.auroraColor[1], 0.0f, 1.0f);

    ui_add_tuner(&ui_ctx, start_x + 400, start_y + 6 * spacing, btn_w, btn_h,
                 "Aurora B", &g_skyParams.auroraColor[2], 0.0f, 1.0f);

    ui_add_tuner(&ui_ctx, start_x + 400, start_y + 0 * spacing, btn_w, btn_h,
                 "Nebula Night R", &g_skyParams.nebulaColor1[0], 0.0f, 1.0f);

    ui_add_tuner(&ui_ctx, start_x + 400, start_y + 1 * spacing, btn_w, btn_h,
                 "Nebula Night G", &g_skyParams.nebulaColor1[1], 0.0f, 1.0f);

    ui_add_tuner(&ui_ctx, start_x + 400, start_y + 2 * spacing, btn_w, btn_h,
                 "Nebula Night B", &g_skyParams.nebulaColor1[2], 0.0f, 1.0f);

    // === Left column - Parameters ===
    ui_add_tuner(&ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Aurora Intensity", &g_skyParams.auroraIntensity, 0.0f, 2.0f);

    ui_add_tuner(&ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Aurora Speed", &g_skyParams.auroraSpeed, 0.0f, 5.0f);

    ui_add_tuner(&ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Brightness", &g_skyParams.overallBrightness, 0.0f, 5.0f);

    ui_add_tuner(&ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Cycle Speed", &g_skyParams.cycleSpeed, 0.0f, 2.0f);

    ui_add_tuner(&ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Nebula Intensity", &g_skyParams.nebulaIntensity, 0.0f, 3.0f);

    ui_add_tuner(&ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Nebula Speed", &g_skyParams.nebulaSpeed, 0.0f, 10.0f);

    ui_add_tuner(&ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Nebula Scale", &g_skyParams.nebulaScale, 0.5f, 10.0f);

    ui_add_tuner(&ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Nebula Layers", &g_skyParams.nebulaLayerCount, 1.0f, 8.0f);

    ui_add_tuner(&ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Star Count", &g_skyParams.starCount, 0.0f, 2500.0f);

    ui_add_tuner(&ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Star Brightness", &g_skyParams.starBrightness, 0.0f, 5.0f);

    ui_add_tuner(&ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Star Twinkle", &g_skyParams.starTwinkleSpeed, 0.0f, 20.0f);

    ui_add_tuner(&ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Star Size", &g_skyParams.starSize, 0.001f, 0.3f);

    ui_add_tuner(&ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Time of Day", &g_skyParams.timeOfDay, 0.0f, 1.0f);

    ui_add_tuner(&ui_ctx, start_x, start_y + idx++ * spacing, btn_w, btn_h,
                 "Vignette", &g_skyParams.vignetteStrength, 0.0f, 1.5f);
}

static void create_sky_ubo(void)
{
    VkDeviceSize size = sizeof(SkyUBO);
    create_vulkan_buffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         &skyUBOBuffer, &skyUBOMemory);
}

static void create_sky_descriptors(void)
{
    VkDescriptorSetLayoutBinding bindings[2] = {0};

    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;

    vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &skyDescriptorSetLayout);

    VkDescriptorPoolSize poolSize = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 };
    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    vkCreateDescriptorPool(device, &poolInfo, NULL, &skyDescriptorPool);

    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = skyDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &skyDescriptorSetLayout;
    vkAllocateDescriptorSets(device, &allocInfo, &skyDescriptorSet);

    // Initial write
    VkDescriptorBufferInfo bufInfo = { skyUBOBuffer, 0, sizeof(SkyUBO) };
    VkWriteDescriptorSet write = {0};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = skyDescriptorSet;
    write.dstBinding = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &bufInfo;
    vkUpdateDescriptorSets(device, 1, &write, 0, NULL);
}

void create_sky_pipeline(void)
{
    size_t vert_size, frag_size;
    uint32_t* vert_code = load_spirv("../../shaders/sky.vert.spv", &vert_size);
    uint32_t* frag_code = load_spirv("../../shaders/sky.frag.spv", &frag_size);
    if (!vert_code || !frag_code) {
        printf("Failed to load sky shaders!\n");
        exit(1);
    }

    VkShaderModuleCreateInfo vertCreate = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                            .codeSize = vert_size, .pCode = vert_code };
    vkCreateShaderModule(device, &vertCreate, NULL, &vertShaderModule);
    free(vert_code);

    VkShaderModuleCreateInfo fragCreate = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                            .codeSize = frag_size, .pCode = frag_code };
    vkCreateShaderModule(device, &fragCreate, NULL, &fragShaderModule);
    free(frag_code);

    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = vertShaderModule, .pName = "main" },
        { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = fragShaderModule, .pName = "main" }
    };

    VkPipelineVertexInputStateCreateInfo vertexInput = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
    };

    VkPipelineViewportStateCreateInfo viewportState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                                                        .viewportCount = 1, .scissorCount = 1 };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL, .lineWidth = 1.0f, .cullMode = VK_CULL_MODE_NONE
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    VkPipelineDepthStencilStateCreateInfo depthStencil = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_FALSE,
        .depthWriteEnable = VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_ALWAYS,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE, // Explicitly false
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1, .pAttachments = &colorBlendAttachment
    };

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2, .pDynamicStates = dynamicStates
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &skyDescriptorSetLayout;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &skyPipelineLayout) != VK_SUCCESS) {
        printf("Failed to create sky pipeline layout\n");
        exit(1);
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInput,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = skyPipelineLayout,
        .renderPass = renderPass,
        .subpass = 0
    };

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &skyPipeline) != VK_SUCCESS) {
        printf("Failed to create sky pipeline\n");
        exit(1);
    }

    printf("Sky pipeline created successfully\n");
}

void sky_init(void)
{
    create_sky_ubo();
    create_sky_descriptors();
    create_sky_pipeline();
    setup_sky_tuners();
}

void sky_cleanup(void)
{
    vkDeviceWaitIdle(device);
    vkDestroyPipeline(device, skyPipeline, NULL);
    vkDestroyPipelineLayout(device, skyPipelineLayout, NULL);
    vkDestroyShaderModule(device, vertShaderModule, NULL);
    vkDestroyShaderModule(device, fragShaderModule, NULL);
    vkDestroyBuffer(device, skyUBOBuffer, NULL);
    vkFreeMemory(device, skyUBOMemory, NULL);
    vkDestroyDescriptorSetLayout(device, skyDescriptorSetLayout, NULL);
    vkDestroyDescriptorPool(device, skyDescriptorPool, NULL);
}

void sky_update(void)
{
    // Auto cycle
    if (g_skyParams.cycleSpeed > 0.0f) {
        g_skyParams.timeOfDay += 0.0005f * g_skyParams.cycleSpeed;
        if (g_skyParams.timeOfDay > 1.0f) g_skyParams.timeOfDay -= 1.0f;
    }

    // Day/night blend
    float blend = (sinf(g_skyParams.timeOfDay * 6.283185f - 1.57f) + 1.0f) * 0.5f;
    g_skyParams.dayNightBlend = blend;

    static float accumTime = 0.0f;
    accumTime += 0.016f;

    skyUBOData.time = accumTime;
    skyUBOData.yaw = camera.yaw * 0.0174532925f;
    skyUBOData.pitch = camera.pitch * 0.0174532925f;

    skyUBOData.timeOfDay = g_skyParams.timeOfDay;
    skyUBOData.dayNightBlend = g_skyParams.dayNightBlend;

    skyUBOData.nebulaScale = g_skyParams.nebulaScale;
    skyUBOData.nebulaIntensity = g_skyParams.nebulaIntensity;
    skyUBOData.nebulaLayerCount = g_skyParams.nebulaLayerCount;
    skyUBOData.nebulaSpeed = g_skyParams.nebulaSpeed;

    skyUBOData.starCount = g_skyParams.starCount;
    skyUBOData.starBrightness = g_skyParams.starBrightness;
    skyUBOData.starTwinkleSpeed = g_skyParams.starTwinkleSpeed;
    skyUBOData.starSize = g_skyParams.starSize;

    skyUBOData.auroraIntensity = g_skyParams.auroraIntensity;
    skyUBOData.auroraSpeed = g_skyParams.auroraSpeed;

    // Clear and Copy Colors (Ensure the 4th float is 1.0 or 0.0)
    memset(skyUBOData.nebulaColorNight, 0, sizeof(float)*4);
    memcpy(skyUBOData.nebulaColorNight, g_skyParams.nebulaColor1, sizeof(float)*3);

    memset(skyUBOData.nebulaColorDay, 0, sizeof(float)*4);
    memcpy(skyUBOData.nebulaColorDay, g_skyParams.nebulaColor2, sizeof(float)*3);

    memset(skyUBOData.auroraColor, 0, sizeof(float)*4);
    memcpy(skyUBOData.auroraColor, g_skyParams.auroraColor, sizeof(float)*3);

    skyUBOData.vignetteStrength = g_skyParams.vignetteStrength;
    skyUBOData.overallBrightness = g_skyParams.overallBrightness;
    memcpy(skyUBOData.inverseView, cameraUBOData.inverseView, 16 * sizeof(float));
    // Upload
    void* data;
    vkMapMemory(device, skyUBOMemory, 0, sizeof(SkyUBO), 0, &data);
    memcpy(data, &skyUBOData, sizeof(SkyUBO));
    vkUnmapMemory(device, skyUBOMemory);
}

void sky_draw(VkCommandBuffer cmd)
{
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, skyPipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, skyPipelineLayout, 0, 1, &skyDescriptorSet, 0, NULL);

    VkViewport viewport = {0, 0, (float)swapchainExtent.width, (float)swapchainExtent.height, 0.0f, 1.0f};
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor = {{0,0}, swapchainExtent};
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    vkCmdDraw(cmd, 4, 1, 0, 0);
}