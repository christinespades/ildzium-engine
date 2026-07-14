#include "pch.h"
#ifndef __EMSCRIPTEN__
	#include "renderer_debug_vk.h"

	extern VkRenderPass renderPass;
	DebugRenderer g_renderer_debug;

	static void create_debug_quad(void)
	{
	    VkDeviceSize size = sizeof(debug_quad_vertices);

	    create_vulkan_buffer(
	        size,
	        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	        &g_renderer_debug.quadVertexBuffer,
	        &g_renderer_debug.quadVertexMemory);

	    void* data;

	    vkMapMemory(
	        vk_device,
	        g_renderer_debug.quadVertexMemory,
	        0,
	        size,
	        0,
	        &data);

	    memcpy(data, debug_quad_vertices, size);

	    vkUnmapMemory(
	        vk_device,
	        g_renderer_debug.quadVertexMemory);
	}

	void create_debug_instance_and_primitive_buffers() {
	    VkDeviceSize size =
	        MAX_DEBUG_LINES *
	        sizeof(DebugLineInstance);

	    for(uint32_t i = 0;
	        i < RENDER_MAX_FRAMES_IN_FLIGHT;
	        i++)
	    {
	        create_vulkan_buffer(
	            size,
	            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
			    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	            &g_renderer_debug.instanceBuffers[i],
	            &g_renderer_debug.instanceMemory[i]);

	        vkMapMemory(
	            vk_device,
	            g_renderer_debug.instanceMemory[i],
	            0,
	            size,
	            0,
	            &g_renderer_debug.mappedInstanceBuffers[i]);
	    }

        size =
        MAX_DEBUG_PRIMITIVES *
        sizeof(DebugPrimitiveCommand);

	    for(uint32_t i = 0;
	        i < RENDER_MAX_FRAMES_IN_FLIGHT;
	        i++)
	    {
	        create_vulkan_buffer(
	            size,
	            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
	            &g_renderer_debug.primitiveBuffers[i],
	            &g_renderer_debug.primitiveMemory[i]);

	        vkMapMemory(
	            vk_device,
	            g_renderer_debug.primitiveMemory[i],
	            0,
	            size,
	            0,
	            &g_renderer_debug.mappedPrimitiveBuffers[i]);
	    }
	}

	void create_debug_pipeline()
	{
		VkShaderModule debugVertModule, debugFragModule;
	    load_shader_pair("debug", &debugVertModule, &debugFragModule);

	    VkPipelineShaderStageCreateInfo stages[2] = {0};
	    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	    stages[0].module = debugVertModule;
	    stages[0].pName = "main";

	    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	    stages[1].module = debugFragModule;
	    stages[1].pName = "main";

		VkVertexInputBindingDescription bindings[2] = {0};

		// Static quad
		bindings[0].binding = 0;
		bindings[0].stride = sizeof(DebugQuadVertex);
		bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


		// Per-line instance data
		bindings[1].binding = 1;
		bindings[1].stride = sizeof(DebugLineInstance);
		bindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

		VkVertexInputAttributeDescription attrs[5] = {0};
		// Quad position
		attrs[0].location = 0;
		attrs[0].binding  = 0;
		attrs[0].format   = VK_FORMAT_R32G32_SFLOAT;
		attrs[0].offset   = offsetof(DebugQuadVertex, x);

		// Line start
		attrs[1].location = 1;
		attrs[1].binding  = 1;
		attrs[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
		attrs[1].offset   = offsetof(DebugLineInstance,start);

		// Line end
		attrs[2].location = 2;
		attrs[2].binding  = 1;
		attrs[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
		attrs[2].offset   = offsetof(DebugLineInstance,end);

		// Width
		attrs[3].location = 3;
		attrs[3].binding  = 1;
		attrs[3].format   = VK_FORMAT_R32_SFLOAT;
		attrs[3].offset   = offsetof(DebugLineInstance,start) + sizeof(float)*3;

		// Color
		attrs[4].location = 4;
		attrs[4].binding  = 1;
		attrs[4].format   = VK_FORMAT_R32G32B32_SFLOAT;
		attrs[4].offset   = offsetof(DebugLineInstance,color);

	    VkPipelineVertexInputStateCreateInfo vertexInput = {0};
	    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInput.vertexBindingDescriptionCount = 2;
		vertexInput.pVertexBindingDescriptions = bindings;

		vertexInput.vertexAttributeDescriptionCount = 5;
		vertexInput.pVertexAttributeDescriptions = attrs;

	    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
	    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	    inputAssembly.primitiveRestartEnable = VK_FALSE;

	    VkPipelineViewportStateCreateInfo viewportState = {0};
	    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	    viewportState.viewportCount = 1;
	    viewportState.scissorCount = 1;

	    VkPipelineRasterizationStateCreateInfo raster = {0};
	    raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	    raster.polygonMode = VK_POLYGON_MODE_FILL;
	    raster.lineWidth = 1.0f;
	    raster.cullMode = VK_CULL_MODE_NONE;

	    VkPipelineMultisampleStateCreateInfo ms = {0};
	    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	    ms.rasterizationSamples = msaaSamples;

	    VkPipelineDepthStencilStateCreateInfo depthStencil = {0};
	    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	    depthStencil.depthTestEnable = VK_TRUE;
	    depthStencil.depthWriteEnable = VK_FALSE; // Don't disrupt regular object depth!
	    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

	    VkPipelineColorBlendAttachmentState blendAttach = {0};
	    blendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	    blendAttach.blendEnable = VK_TRUE;
		blendAttach.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttach.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttach.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttach.alphaBlendOp = VK_BLEND_OP_ADD;

	    VkPipelineColorBlendStateCreateInfo blend = {0};
	    blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	    blend.attachmentCount = 1;
	    blend.pAttachments = &blendAttach;

	    VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	    VkPipelineDynamicStateCreateInfo dyn = {0};
	    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	    dyn.dynamicStateCount = 2;
	    dyn.pDynamicStates = dynStates;

	    VkGraphicsPipelineCreateInfo pipeInfo = {0};
	    pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	    pipeInfo.stageCount = 2;
	    pipeInfo.pStages = stages;
	    pipeInfo.pVertexInputState = &vertexInput;
	    pipeInfo.pInputAssemblyState = &inputAssembly;
	    pipeInfo.pViewportState = &viewportState;
	    pipeInfo.pRasterizationState = &raster;
	    pipeInfo.pMultisampleState = &ms;
	    pipeInfo.pDepthStencilState = &depthStencil;
	    pipeInfo.pColorBlendState = &blend;
	    pipeInfo.pDynamicState = &dyn;
	    pipeInfo.layout = modelPipelineLayout; 
	    pipeInfo.renderPass = renderPass;
	    pipeInfo.subpass = 0;

	    if (vkCreateGraphicsPipelines(vk_device, VK_NULL_HANDLE, 1, &pipeInfo, NULL, &g_renderer_debug.pipeline) != VK_SUCCESS) {
	        LOGE("Failed to create debug pipeline");
	    }
	    LOGI("Created debug pipeline");
	}

	void create_debug_pipelines()
	{
		create_debug_pipeline();
		create_debug_compute_pipeline();
	}

	void renderer_debug_init()
	{
		memset(&g_renderer_debug, 0, sizeof(g_renderer_debug));
		create_debug_quad();
		create_debug_instance_and_primitive_buffers();
	    create_debug_pipelines();
	}

	void renderer_debug_destroy()
	{
	    if(g_renderer_debug.pipeline != VK_NULL_HANDLE)
	    {
	        vkDestroyPipeline(
	            vk_device,
	            g_renderer_debug.pipeline,
	            NULL
	        );
	    }


	    if(g_renderer_debug.quadVertexBuffer)
	    {
	        vkDestroyBuffer(
	            vk_device,
	            g_renderer_debug.quadVertexBuffer,
	            NULL
	        );

	        vkFreeMemory(
	            vk_device,
	            g_renderer_debug.quadVertexMemory,
	            NULL
	        );
	    }


	    for(uint32_t i = 0;
	        i < get_param_color(PARAM_RENDER_MAX_FRAMES_IN_FLIGHT);
	        i++)
	    {
	        if(g_renderer_debug.instanceBuffers[i])
	        {
	            vkUnmapMemory(
	                vk_device,
	                g_renderer_debug.instanceMemory[i]
	            );

	            vkDestroyBuffer(
	                vk_device,
	                g_renderer_debug.instanceBuffers[i],
	                NULL
	            );

	            vkFreeMemory(
	                vk_device,
	                g_renderer_debug.instanceMemory[i],
	                NULL
	            );
	        }
	        if(g_renderer_debug.primitiveBuffers[i])
	        {
	            vkUnmapMemory(
	                vk_device,
	                g_renderer_debug.primitiveMemory[i]
	            );

	            vkDestroyBuffer(
	                vk_device,
	                g_renderer_debug.primitiveBuffers[i],
	                NULL
	            );

	            vkFreeMemory(
	                vk_device,
	                g_renderer_debug.primitiveMemory[i],
	                NULL
	            );
	        }
	    }
	}
#endif