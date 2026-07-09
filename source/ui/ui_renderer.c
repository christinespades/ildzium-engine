#include "pch.h"
#include "ui/ui_renderer.h"

uint32_t* ui_framebuffer = NULL;

#ifndef __EMSCRIPTEN__
    extern VkDevice vk_device;
    extern VkPhysicalDevice physicalDevice;
    extern VkExtent2D swapchainExtent;
    extern VkCommandPool commandPool;
    extern VkQueue graphicsQueue;
    extern VkRenderPass renderPass;
    extern uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags);
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
#else
    #include "rendering/renderer_webgpu.h"
    extern WGPUDevice device;
    extern WGPUQueue queue;
    extern WGPUTextureFormat swapchainFormat;

    WGPURenderPipeline uiPipeline      = NULL;
    WGPUBindGroup      uiBindGroup     = NULL;
    WGPUBuffer         uiVertexBuffer  = NULL;
    WGPUTexture        uiTexture       = NULL;
    WGPUTextureView    uiTextureView   = NULL;
    WGPUSampler        uiSampler       = NULL;

    static uint32_t currentUiWidth = 1280;
    static uint32_t currentUiHeight = 720;

    // when size changes
    static void create_ui_texture(uint32_t width, uint32_t height)
{
    if (uiTexture) wgpuTextureRelease(uiTexture);
    if (uiTextureView) wgpuTextureViewRelease(uiTextureView);

    WGPUTextureDescriptor texDesc = {0};
    texDesc.usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding;
    texDesc.dimension = WGPUTextureDimension_2D;
    texDesc.size = (WGPUExtent3D){width, height, 1};
    texDesc.format = WGPUTextureFormat_RGBA8Unorm;
    texDesc.mipLevelCount = 1;
    texDesc.sampleCount = 1;

    uiTexture = wgpuDeviceCreateTexture(device, &texDesc);

    WGPUTextureViewDescriptor viewDesc = {0};
    viewDesc.format = WGPUTextureFormat_RGBA8Unorm;
    viewDesc.dimension = WGPUTextureViewDimension_2D;
    viewDesc.mipLevelCount = 1;
    viewDesc.arrayLayerCount = 1;
    viewDesc.aspect = WGPUTextureAspect_All;

    uiTextureView = wgpuTextureCreateView(uiTexture, &viewDesc);

    currentUiWidth = width;
    currentUiHeight = height;
}
#endif
    typedef struct { float x, y, u, v; } Vertex;
    Vertex quadVerts[4] = {
        {-1.f, -1.f, 0.f, 1.f},   // bottom-left in NDC
        { 1.f, -1.f, 1.f, 1.f},
        {-1.f,  1.f, 0.f, 0.f},   // top-left in NDC
        { 1.f,  1.f, 1.f, 0.f}
    };

#ifndef __EMSCRIPTEN__
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
        allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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

        VkBufferCreateInfo bufInfo = {0};
        bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufInfo.size = size;
        bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateBuffer(vk_device, &bufInfo, NULL, &uiVertexBuffer);

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(vk_device, uiVertexBuffer, &memReq);

        VkMemoryAllocateInfo alloc = {0};
        alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc.allocationSize = memReq.size;
        alloc.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkAllocateMemory(vk_device, &alloc, NULL, &uiVertexBufferMemory);
        vkBindBufferMemory(vk_device, uiVertexBuffer, uiVertexBufferMemory, 0);

        void* data;
        vkMapMemory(vk_device, uiVertexBufferMemory, 0, size, 0, &data);
        memcpy(data, quadVertices, size);
        vkUnmapMemory(vk_device, uiVertexBufferMemory);
    }

    void ui_renderer_resize()
    {
        // Make sure GPU is not using old resources
        vkDeviceWaitIdle(vk_device);

        // --- Destroy old image resources ---
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

        // --- Recreate with new swapchain extent ---
        create_ui_image();

        // --- Update descriptor set (IMPORTANT) ---
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

#endif

    void ui_renderer_upload(uint32_t* ui_pixels, uint32_t width, uint32_t height) {
    #ifndef __EMSCRIPTEN__
        VkDeviceSize bufferSize = (VkDeviceSize)width * height * sizeof(uint32_t);
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;

        VkBufferCreateInfo bufInfo = {0};
        bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufInfo.size = bufferSize;
        bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateBuffer(vk_device, &bufInfo, NULL, &stagingBuffer);

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(vk_device, stagingBuffer, &memReq);

        VkMemoryAllocateInfo allocInfo = {0};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkAllocateMemory(vk_device, &allocInfo, NULL, &stagingMemory);
        vkBindBufferMemory(vk_device, stagingBuffer, stagingMemory, 0);

        // 2. Copy CPU data to staging buffer
        void* data;
        vkMapMemory(vk_device, stagingMemory, 0, bufferSize, 0, &data);
        memcpy(data, ui_pixels, bufferSize);
        vkUnmapMemory(vk_device, stagingMemory);

        // 3. Record command buffer to copy staging -> GPU image
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
        copyRegion.imageExtent = (VkExtent3D){width, height, 1};

        vkCmdCopyBufferToImage(cmd, stagingBuffer, uiImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        // Transition to SHADER_READ_ONLY_OPTIMAL for sampling
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
    #else
        if (!ui_pixels) {
            printf("No UI pixels\n");
            return;
        }

        // Recreate texture if size changed
        if (width != currentUiWidth || height != currentUiHeight) {
            create_ui_texture(width, height);
            // Recreate bind group with new texture view
            // (for simplicity we recreate bind group here - you can optimize later)
            if (uiBindGroup) wgpuBindGroupRelease(uiBindGroup);
            // ... recreate bind group same as in init (copy the code or make a helper)
            // For now, call ui_renderer_init() again is acceptable for UI
            ui_renderer_init();   // simple but works
        }
        WGPUTexelCopyTextureInfo dst = {
            .texture = uiTexture,
            .mipLevel = 0,
            .origin = {0, 0, 0},
            .aspect = WGPUTextureAspect_All
        };

        WGPUTexelCopyBufferLayout layout = {
            .offset = 0,
            .bytesPerRow = width * 4,
            .rowsPerImage = height
        };

        WGPUExtent3D size = { 
            .width = width, 
            .height = height, 
            .depthOrArrayLayers = 1 
        };

        wgpuQueueWriteTexture(queue, &dst, ui_pixels, (size_t)width * height * 4, &layout, &size);
    #endif
    }

#ifndef __EMSCRIPTEN__
    static VkShaderModule uiVertModule;
    static VkShaderModule uiFragModule;

    void create_ui_pipeline() {
        size_t vert_size, frag_size;

        uint32_t* vert_code = load_spirv("../../shaders/ui.vert.spv", &vert_size);
        uint32_t* frag_code = load_spirv("../../shaders/ui.frag.spv", &frag_size);

        if (!vert_code || !frag_code) {
            printf("Failed to load UI shaders\n");
            exit(1);
        }

        VkShaderModuleCreateInfo sm = {0};
        sm.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

        sm.codeSize = vert_size;
        sm.pCode = vert_code;
        vkCreateShaderModule(vk_device, &sm, NULL, &uiVertModule);

        sm.codeSize = frag_size;
        sm.pCode = frag_code;
        vkCreateShaderModule(vk_device, &sm, NULL, &uiFragModule);

        free(vert_code);
        free(frag_code);

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
        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

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
#endif
    void ui_renderer_init() {
    #ifndef __EMSCRIPTEN__
    	create_ui_image();
    	create_ui_descriptor();
    	create_ui_pipeline();
    	create_ui_quad();
    #else
    printf("Creating WebGPU UI Renderer...\n");

    // Initial texture (default size)
    create_ui_texture(1280, 720);

    // Sampler
    WGPUSamplerDescriptor sampDesc = {0};
    sampDesc.magFilter = WGPUFilterMode_Nearest;
    sampDesc.minFilter = WGPUFilterMode_Nearest;
    sampDesc.addressModeU = WGPUAddressMode_ClampToEdge;
    sampDesc.addressModeV = WGPUAddressMode_ClampToEdge;
    sampDesc.addressModeW = WGPUAddressMode_ClampToEdge;
    sampDesc.maxAnisotropy = 1;
    uiSampler = wgpuDeviceCreateSampler(device, &sampDesc);

    // Vertex buffer (full-screen quad)
    float quadVerts[] = {
        -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 0.0f
    };

    WGPUBufferDescriptor vbDesc = {0};
    vbDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    vbDesc.size = sizeof(quadVerts);
    uiVertexBuffer = wgpuDeviceCreateBuffer(device, &vbDesc);
    wgpuQueueWriteBuffer(queue, uiVertexBuffer, 0, quadVerts, sizeof(quadVerts));

    // === Bind Group Layout ===
    WGPUBindGroupLayoutEntry entries[2] = {0};

    // Texture Binding
    entries[0].binding = 0;
    entries[0].visibility = WGPUShaderStage_Fragment;
    entries[0].texture.sampleType = WGPUTextureSampleType_Float;
    entries[0].texture.viewDimension = WGPUTextureViewDimension_2D;

    // Sampler Binding (MISSING IN YOUR ORIGINAL CODE)
    entries[1].binding = 1;
    entries[1].visibility = WGPUShaderStage_Fragment;
    entries[1].sampler.type = WGPUSamplerBindingType_Filtering;

    WGPUBindGroupLayoutDescriptor layoutDesc = {0};
    layoutDesc.entryCount = 2; // Change to 2
    layoutDesc.entries = entries;

    WGPUBindGroupLayout bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &layoutDesc);

    // === Pipeline Layout ===
    WGPUPipelineLayoutDescriptor plDesc = {0};
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = &bindGroupLayout;
    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &plDesc);

    // === Shader ===
    const char* wgsl = 
        "@group(0) @binding(0) var uiTexture : texture_2d<f32>;\n"
        "@group(0) @binding(1) var uiSampler : sampler;\n"

        "struct VSOut {\n"
        "    @builtin(position) pos : vec4<f32>,\n"
        "    @location(0) uv : vec2<f32>\n"
        "};\n"

        "@vertex fn vs_main(@location(0) pos : vec2<f32>, @location(1) uv : vec2<f32>) -> VSOut {\n"
        "    var out : VSOut;\n"
        "    out.pos = vec4<f32>(pos, 0.0, 1.0);\n"
        "    out.uv = -uv;\n"
        "    return out;\n"
        "}\n"

        "@fragment fn fs_main(@location(0) uv : vec2<f32>) -> @location(0) vec4<f32> {\n"
        "    return vec4<f32>(1.0, 1.0, 1.0, 1.0);\n"
        "}\n";

    WGPUShaderModule shader = create_shader_module(device, wgsl, "UIShader");

    // Vertex layout (pos + uv)
    WGPUVertexAttribute attrs[2] = {
        { .format = WGPUVertexFormat_Float32x2, .offset = 0,  .shaderLocation = 0 },
        { .format = WGPUVertexFormat_Float32x2, .offset = 8,  .shaderLocation = 1 }
    };
    WGPUVertexBufferLayout vbLayout = {
        .arrayStride = 16,
        .attributeCount = 2,
        .attributes = attrs
    };

    WGPUColorTargetState colorTarget = {0};
    colorTarget.format = swapchainFormat;

    // 1. Define the blend state as a separate variable
    WGPUBlendState blendState = {
        .color = { 
            .operation = WGPUBlendOperation_Add, 
            .srcFactor = WGPUBlendFactor_SrcAlpha, 
            .dstFactor = WGPUBlendFactor_OneMinusSrcAlpha 
        },
        .alpha = { 
            .operation = WGPUBlendOperation_Add, 
            .srcFactor = WGPUBlendFactor_One, 
            .dstFactor = WGPUBlendFactor_Zero 
        }
    };

    // 2. Assign the address of that variable to the target
    colorTarget.blend = &blendState;

    const char* vs_entry = "vs_main";
    const char* fs_entry = "fs_main";

    WGPUFragmentState fragment = {
        .module = shader,
        .entryPoint = (WGPUStringView){.data = fs_entry, .length = strlen(fs_entry)},
        .targetCount = 1,
        .targets = &colorTarget
    };

    WGPUDepthStencilState depthStencil = {0};
    depthStencil.format = WGPUTextureFormat_Depth24Plus;
    depthStencil.depthWriteEnabled = true;
    depthStencil.depthCompare = WGPUCompareFunction_Less;

    WGPURenderPipelineDescriptor desc = {0};
    desc.layout = pipelineLayout;

    desc.vertex.module = shader;
    desc.vertex.entryPoint = (WGPUStringView){.data = vs_entry, .length = strlen(vs_entry)};
    desc.vertex.bufferCount = 1;
    desc.vertex.buffers = &vbLayout;
    desc.fragment = &fragment;
    desc.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
    //desc.primitive.cullMode = WGPUCullMode_None;
    desc.multisample.count = 1;
    desc.depthStencil = &depthStencil;
    desc.label = (WGPUStringView){ .data = "UIPipeline", .length = 10 };

    uiPipeline = wgpuDeviceCreateRenderPipeline(device, &desc);

    // === Create Bind Group ===
    WGPUBindGroupEntry bgEntries[2] = {0};

    // Texture View
    bgEntries[0].binding = 0;
    bgEntries[0].textureView = uiTextureView;

    // Sampler (MISSING IN YOUR ORIGINAL CODE)
    bgEntries[1].binding = 1;
    bgEntries[1].sampler = uiSampler;

    WGPUBindGroupDescriptor bgDesc = {0};
    bgDesc.layout = bindGroupLayout;
    bgDesc.entryCount = 2; // Change to 2
    bgDesc.entries = bgEntries;

    uiBindGroup = wgpuDeviceCreateBindGroup(device, &bgDesc);

    printf("WebGPU UI pipeline + bind group created with alpha blending\n");
    #endif
    }

#ifndef __EMSCRIPTEN__
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

#ifdef __EMSCRIPTEN__
    void ui_renderer_draw(WGPURenderPassEncoder pass)
    {
        if (!uiPipeline || !uiBindGroup) return;
        wgpuRenderPassEncoderSetPipeline(pass, uiPipeline);
        wgpuRenderPassEncoderSetBindGroup(pass, 0, uiBindGroup, 0, NULL);
        wgpuRenderPassEncoderSetVertexBuffer(pass, 0, uiVertexBuffer, 0, sizeof(quadVerts));
        wgpuRenderPassEncoderDraw(pass, 4, 1, 0, 0);
    }

    void ui_renderer_cleanup(void)
    {
        if (uiPipeline) wgpuRenderPipelineRelease(uiPipeline);
        if (uiBindGroup) wgpuBindGroupRelease(uiBindGroup);
        if (uiVertexBuffer) wgpuBufferRelease(uiVertexBuffer);
        if (uiTextureView) wgpuTextureViewRelease(uiTextureView);
        if (uiTexture) wgpuTextureRelease(uiTexture);
        if (uiSampler) wgpuSamplerRelease(uiSampler);
    }
#endif