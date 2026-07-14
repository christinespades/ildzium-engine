#include "pch.h"
#ifndef __EMSCRIPTEN__
    #include "ui/ui_renderer_vk.h"

    extern VkDevice vk_device;
    extern VkPhysicalDevice physicalDevice;
    extern VkExtent2D swapchainExtent;
    extern VkCommandPool commandPool;
    extern VkQueue graphicsQueue;
    extern VkRenderPass renderPass;
    extern uint32_t find_memory_type(uint32_t, VkMemoryPropertyFlags);
    extern uint32_t* load_spirv(const char*, size_t*);

    VkImage uiImage = VK_NULL_HANDLE;
    VkDeviceMemory uiImageMemory = VK_NULL_HANDLE;
    VkImageView uiImageView = VK_NULL_HANDLE;
    VkSampler uiSampler = VK_NULL_HANDLE;
    VkDescriptorSet uiDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout uiDescriptorSetLayout;
    VkDescriptorPool uiDescriptorPool;
    VkPipeline uiPipeline = VK_NULL_HANDLE;
    VkPipelineLayout uiPipelineLayout = VK_NULL_HANDLE;
    VkBuffer uiVertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory uiVertexBufferMemory = VK_NULL_HANDLE;

    typedef struct { float x, y, u, v; } Vertex;
    Vertex quadVerts[4] = {
        {-1.f, -1.f, 0.f, 1.f},   // bottom-left in NDC
        { 1.f, -1.f, 1.f, 1.f},
        {-1.f,  1.f, 0.f, 0.f},   // top-left in NDC
        { 1.f,  1.f, 1.f, 0.f}
    };

    void create_ui_image() {
        VkImageCreateInfo imageInfo = {0};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapchainExtent.width;
        imageInfo.extent.height = swapchainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vkCreateImage(vk_device, &imageInfo, NULL, &uiImage);

        VkMemoryRequirements memReq;
        vkGetImageMemoryRequirements(vk_device, uiImage, &memReq);

        VkMemoryAllocateInfo allocInfo = {0};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = find_memory_type(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkAllocateMemory(vk_device, &allocInfo, NULL, &uiImageMemory);
        vkBindImageMemory(vk_device, uiImage, uiImageMemory, 0);

        // Create image view
        VkImageViewCreateInfo viewInfo = {0};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = uiImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(vk_device, &viewInfo, NULL, &uiImageView);

        // Sampler
        VkSamplerCreateInfo sampInfo = {0};
        sampInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampInfo.magFilter = VK_FILTER_NEAREST;
        sampInfo.minFilter = VK_FILTER_NEAREST;
        sampInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        sampInfo.unnormalizedCoordinates = VK_FALSE;

        vkCreateSampler(vk_device, &sampInfo, NULL, &uiSampler);
    }

    void create_ui_descriptor() {
        VkDescriptorSetLayoutBinding binding = {0};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding.pImmutableSamplers = NULL;

        VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &binding;

        vkCreateDescriptorSetLayout(vk_device, &layoutInfo, NULL, &uiDescriptorSetLayout);

        VkPipelineLayoutCreateInfo layoutCreate = {0};
        layoutCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreate.setLayoutCount = 1;
        layoutCreate.pSetLayouts = &uiDescriptorSetLayout;

        vkCreatePipelineLayout(vk_device, &layoutCreate, NULL, &uiPipelineLayout);

        VkDescriptorPoolSize poolSize = {0};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolInfo = {0};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 1;

        vkCreateDescriptorPool(vk_device, &poolInfo, NULL, &uiDescriptorPool);

        VkDescriptorSetAllocateInfo allocInfo = {0};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = uiDescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &uiDescriptorSetLayout;

        vkAllocateDescriptorSets(vk_device, &allocInfo, &uiDescriptorSet);

        VkDescriptorImageInfo imgInfo = {0};
        imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imgInfo.imageView = uiImageView;
        imgInfo.sampler = uiSampler;

        VkWriteDescriptorSet write = {0};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = uiDescriptorSet;
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;
        write.pImageInfo = &imgInfo;

        vkUpdateDescriptorSets(vk_device, 1, &write, 0, NULL);
    }

    void create_ui_quad() {
        Vertex quadVertices[4] = {
            {-1.0f, -1.0f, 0.0f, 0.0f},  // bottom-left  → UV (0,0) = top-left of texture
            { 1.0f, -1.0f, 1.0f, 0.0f},  // bottom-right → UV (1,0)
            {-1.0f,  1.0f, 0.0f, 1.0f},  // top-left     → UV (0,1) = bottom of texture
            { 1.0f,  1.0f, 1.0f, 1.0f}   // top-right
        };
        VkDeviceSize size = sizeof(quadVertices);
        create_vulkan_buffer_mapped(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &uiVertexBuffer, &uiVertexBufferMemory, quadVertices);
    }

    void ui_renderer_resize()
    {
        vkDeviceWaitIdle(vk_device);
        if (uiImageView) {
            vkDestroyImageView(vk_device, uiImageView, NULL);
            uiImageView = VK_NULL_HANDLE;
        }

        if (uiImage) {
            vkDestroyImage(vk_device, uiImage, NULL);
            uiImage = VK_NULL_HANDLE;
        }

        if (uiImageMemory) {
            vkFreeMemory(vk_device, uiImageMemory, NULL);
            uiImageMemory = VK_NULL_HANDLE;
        }

        create_ui_image();
        VkDescriptorImageInfo imgInfo = {0};
        imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imgInfo.imageView = uiImageView;
        imgInfo.sampler = uiSampler;

        VkWriteDescriptorSet write = {0};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = uiDescriptorSet;
        write.dstBinding = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;
        write.pImageInfo = &imgInfo;

        vkUpdateDescriptorSets(vk_device, 1, &write, 0, NULL);
    }

    void ui_renderer_upload(void) {
        VkDeviceSize bufferSize = (VkDeviceSize)g_width * g_height * sizeof(uint32_t);
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;

        create_vulkan_buffer_mapped(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer, &stagingMemory, g_ui_fb);

        VkCommandBufferAllocateInfo allocCmd = {0};
        allocCmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocCmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocCmd.commandPool = commandPool;
        allocCmd.commandBufferCount = 1;

        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(vk_device, &allocCmd, &cmd);

        VkCommandBufferBeginInfo beginInfo = {0};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmd, &beginInfo);
        VkImageMemoryBarrier barrier = {0};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = uiImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, NULL,
            0, NULL,
            1, &barrier
        );

        VkBufferImageCopy copyRegion = {0};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;
        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageOffset = (VkOffset3D){0,0,0};
        copyRegion.imageExtent = (VkExtent3D){g_width, g_height, 1};

        vkCmdCopyBufferToImage(cmd, stagingBuffer, uiImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, NULL,
            0, NULL,
            1, &barrier
        );

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submit = {0};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;

        vkQueueSubmit(graphicsQueue, 1, &submit, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(vk_device, commandPool, 1, &cmd);
        vkDestroyBuffer(vk_device, stagingBuffer, NULL);
        vkFreeMemory(vk_device, stagingMemory, NULL);
    }

    static VkShaderModule uiVertModule;
    static VkShaderModule uiFragModule;

    void create_ui_pipeline() {
        load_shader_pair("ui", &uiVertModule, &uiFragModule);

        VkPipelineShaderStageCreateInfo stages[2] = {0};

        stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = uiVertModule;
        stages[0].pName = "main";

        stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = uiFragModule;
        stages[1].pName = "main";

        VkVertexInputBindingDescription binding = {0};
        binding.binding = 0;
        binding.stride = sizeof(float) * 4;
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription attrs[2] = {0};

        attrs[0].location = 0;
        attrs[0].binding = 0;
        attrs[0].format = VK_FORMAT_R32G32_SFLOAT;
        attrs[0].offset = 0;

        attrs[1].location = 1;
        attrs[1].binding = 0;
        attrs[1].format = VK_FORMAT_R32G32_SFLOAT;
        attrs[1].offset = sizeof(float) * 2;

        VkPipelineVertexInputStateCreateInfo vertexInput = {0};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &binding;
        vertexInput.vertexAttributeDescriptionCount = 2;
        vertexInput.pVertexAttributeDescriptions = attrs;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

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
        ms.rasterizationSamples = msaaSamples;

        VkPipelineColorBlendAttachmentState blendAttach = {0};
        blendAttach.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;

        blendAttach.blendEnable = VK_TRUE;
        blendAttach.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendAttach.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttach.colorBlendOp = VK_BLEND_OP_ADD;

        blendAttach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttach.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo blend = {0};
        blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blend.attachmentCount = 1;
        blend.pAttachments = &blendAttach;

        VkDynamicState dynStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dyn = {0};
        dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dyn.dynamicStateCount = 2;
        dyn.pDynamicStates = dynStates;

        VkGraphicsPipelineCreateInfo pipe = {0};
        pipe.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipe.stageCount = 2;
        pipe.pStages = stages;
        pipe.pVertexInputState = &vertexInput;
        pipe.pInputAssemblyState = &inputAssembly;
        pipe.pViewportState = &viewportState;
        pipe.pRasterizationState = &raster;
        pipe.pMultisampleState = &ms;
        pipe.pColorBlendState = &blend;
        pipe.pDynamicState = &dyn;
        pipe.layout = uiPipelineLayout;
        pipe.renderPass = renderPass;
        pipe.subpass = 0;

    	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    	depthStencil.depthTestEnable = VK_FALSE;
    	depthStencil.depthWriteEnable = VK_FALSE;
    	depthStencil.depthBoundsTestEnable = VK_FALSE;
    	depthStencil.stencilTestEnable = VK_FALSE;

    	pipe.pDepthStencilState = &depthStencil;

        if (vkCreateGraphicsPipelines(vk_device, VK_NULL_HANDLE, 1, &pipe, NULL, &uiPipeline) != VK_SUCCESS) {
            printf("Failed to create UI pipeline\n");
            exit(1);
        }
    }

    void ui_renderer_init()
    {
    	create_ui_image();
    	create_ui_descriptor();
    	create_ui_pipeline();
    	create_ui_quad();
    }

    void ui_renderer_draw(VkCommandBuffer cmd) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, uiPipeline);
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, &uiVertexBuffer, offsets);
        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            uiPipelineLayout,
            0, 1,
            &uiDescriptorSet,
            0, NULL
        );
        VkViewport vp = {0};
        vp.x = 0.0f;
        vp.y = 0.0f;
        vp.width = (float)swapchainExtent.width;
        vp.height = (float)swapchainExtent.height;
        vp.minDepth = 0.0f;
        vp.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &vp);
        VkRect2D scissor = {0};
        scissor.offset = (VkOffset2D){0, 0};
        scissor.extent = swapchainExtent;
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        vkCmdDraw(cmd, 4, 1, 0, 0);
    }
#endif