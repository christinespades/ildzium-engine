#include "pch.h"
#include "renderer_debug_compute_vk.h"

void create_debug_compute_pipeline()
{
    load_compute_shader(
        "debug_geometry",
        &g_renderer_debug.computeModule);

    VkDescriptorSetLayoutBinding bindings[3] = {0};

    bindings[0].binding = 0;
    bindings[0].descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags =
        VK_SHADER_STAGE_COMPUTE_BIT;

    bindings[1].binding = 1;
    bindings[1].descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags =
        VK_SHADER_STAGE_COMPUTE_BIT;

	bindings[2].binding = 2;
	bindings[2].descriptorType =
	    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[2].descriptorCount = 1;
	bindings[2].stageFlags =
	    VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 3;
    layoutInfo.pBindings = bindings;

    vkCreateDescriptorSetLayout(
        vk_device,
        &layoutInfo,
        NULL,
        &g_renderer_debug.computeSetLayout);

    VkPushConstantRange push = {0};
    push.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push.offset = 0;
    push.size = sizeof(uint32_t);

    VkPipelineLayoutCreateInfo layout = {0};
    layout.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    layout.setLayoutCount = 1;
    layout.pSetLayouts = &g_renderer_debug.computeSetLayout;

    layout.pushConstantRangeCount = 1;
    layout.pPushConstantRanges = &push;

    vkCreatePipelineLayout(
        vk_device,
        &layout,
        NULL,
        &g_renderer_debug.computePipelineLayout);

    VkPipelineShaderStageCreateInfo stage = {0};

    stage.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage.module = g_renderer_debug.computeModule;
    stage.pName = "main";

    VkComputePipelineCreateInfo pipeline = {0};

    pipeline.sType =
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

    pipeline.stage = stage;
    pipeline.layout = g_renderer_debug.computePipelineLayout;

    if(vkCreateComputePipelines(
        vk_device,
        VK_NULL_HANDLE,
        1,
        &pipeline,
        NULL,
        &g_renderer_debug.computePipeline) != VK_SUCCESS)
    {
        LOGE("Failed to create debug compute pipeline");
    }

	VkDescriptorPoolSize sizes[2] = {0};

	sizes[0].type =
	    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	sizes[0].descriptorCount = 2 * RENDER_MAX_FRAMES_IN_FLIGHT;

	sizes[1].type =
	    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sizes[1].descriptorCount = 1 * RENDER_MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolCreateInfo pool={0};

	pool.sType =
	    VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool.poolSizeCount = 2;
	pool.pPoolSizes = sizes;
	pool.maxSets=RENDER_MAX_FRAMES_IN_FLIGHT;

	vkCreateDescriptorPool(
	    vk_device,
	    &pool,
	    NULL,
	    &g_renderer_debug.computeDescriptorPool);

	VkDescriptorSetLayout layouts[RENDER_MAX_FRAMES_IN_FLIGHT];

	for(uint32_t i = 0; i < RENDER_MAX_FRAMES_IN_FLIGHT; i++)
	{
	    layouts[i] = g_renderer_debug.computeSetLayout;
	}

	VkDescriptorSetAllocateInfo alloc = {0};
	alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc.descriptorPool = g_renderer_debug.computeDescriptorPool;
	alloc.descriptorSetCount = RENDER_MAX_FRAMES_IN_FLIGHT;
	alloc.pSetLayouts = layouts;

	VK_CHECK(vkAllocateDescriptorSets(
	    vk_device,
	    &alloc,
	    g_renderer_debug.computeDescriptorSets));
}

void renderer_debug_update_compute_descriptors(uint32_t currentFrame) {
	VkDescriptorBufferInfo primitiveInfo={0};

	primitiveInfo.buffer =
	    g_renderer_debug.primitiveBuffers[currentFrame];

	primitiveInfo.offset=0;
	primitiveInfo.range=VK_WHOLE_SIZE;

	VkDescriptorBufferInfo lineInfo={0};

	lineInfo.buffer =
	    g_renderer_debug.instanceBuffers[currentFrame];

	lineInfo.offset=0;
	lineInfo.range=VK_WHOLE_SIZE;

	VkDescriptorBufferInfo cameraInfo = {
	    .buffer = cameraUBOBuffer,
	    .offset = 0,
	    .range = sizeof(CameraUBO)
	};

	VkWriteDescriptorSet writes[3]={0};

	writes[0].sType =
	    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet =
	    g_renderer_debug.computeDescriptorSets[currentFrame];
	writes[0].dstBinding=0;
	writes[0].descriptorCount=1;
	writes[0].descriptorType=
	    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writes[0].pBufferInfo=
	    &primitiveInfo;

	writes[1].sType =
	    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[1].dstSet =
	    g_renderer_debug.computeDescriptorSets[currentFrame];
	writes[1].dstBinding=1;
	writes[1].descriptorCount=1;
	writes[1].descriptorType=
	    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writes[1].pBufferInfo=
	    &lineInfo;

	writes[2].sType =
	    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[2].dstSet =
	    g_renderer_debug.computeDescriptorSets[currentFrame];
	writes[2].dstBinding = 2;
	writes[2].descriptorCount = 1;
	writes[2].descriptorType =
	    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[2].pBufferInfo =
	    &cameraInfo;

	vkUpdateDescriptorSets(
	    vk_device,
	    3,
	    writes,
	    0,
	    NULL);

	//LOGI("%p", g_renderer_debug.primitiveBuffers[currentFrame]);
	//LOGI("%p", g_renderer_debug.instanceBuffers[currentFrame]);
}