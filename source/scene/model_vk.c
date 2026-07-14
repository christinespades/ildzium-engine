#include "pch.h"

#ifndef __EMSCRIPTEN__
    #include "scene/model_vk.h"

    ModelSystem g_model_system = {0};
    extern uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    extern VkDevice vk_device;
    extern VkPipeline modelPipeline;
    extern VkPipelineLayout modelPipelineLayout;
    extern VkDescriptorSet modelDescriptorSet;
    extern void create_vulkan_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                     VkBuffer* buffer, VkDeviceMemory* memory);
    extern VkRenderPass renderPass;
    extern VkShaderModule modelVertModule;
    extern VkShaderModule modelFragModule;
    extern VkDescriptorSetLayout modelDescriptorSetLayout;

    VkDescriptorSetLayout modelDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool modelDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet modelDescriptorSet = VK_NULL_HANDLE;
    VkPipeline modelPipeline = VK_NULL_HANDLE;
    VkPipelineLayout modelPipelineLayout = VK_NULL_HANDLE;
    VkShaderModule modelVertModule = VK_NULL_HANDLE;
    VkShaderModule modelFragModule = VK_NULL_HANDLE;

    void create_model_pipeline(void)
    {
        load_shader_pair("model", &modelVertModule, &modelFragModule);

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
        raster.cullMode = VK_CULL_MODE_BACK_BIT;
        raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo ms = {0};
        ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        ms.rasterizationSamples = msaaSamples;

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

        if (vkCreateGraphicsPipelines(vk_device, VK_NULL_HANDLE, 1, &pipeInfo, NULL, &modelPipeline) != VK_SUCCESS) {
            LOGE("Failed to create model pipeline");
            exit(1);
        }
    }

    void create_model_descriptors(void)
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

        // --- Descriptor set layout ---
        VkDescriptorSetLayoutCreateInfo setLayoutInfo = {0};
        setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        setLayoutInfo.bindingCount = 2;
        setLayoutInfo.pBindings = bindings;

        vkCreateDescriptorSetLayout(vk_device, &setLayoutInfo, NULL, &modelDescriptorSetLayout);

        // --- Pipeline layout ---
        VkDescriptorSetLayout setLayouts[2] = {
            lightDescriptorSetLayout,
            modelDescriptorSetLayout
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 2;
        pipelineLayoutInfo.pSetLayouts = setLayouts;

        if (vkCreatePipelineLayout(vk_device, &pipelineLayoutInfo, NULL, &modelPipelineLayout) != VK_SUCCESS) {
            LOGE("Failed to create model pipeline layout");
            exit(1);
        }

        // Increase the maxSets pool size capacity just in case
        VkDescriptorPoolSize poolSizes[2] = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4},  
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}
        };
        VkDescriptorPoolCreateInfo poolInfo = {0};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 2;
        poolInfo.pPoolSizes = poolSizes;
        poolInfo.maxSets = 4; // Allow plenty of headspace
        vkCreateDescriptorPool(vk_device, &poolInfo, NULL, &modelDescriptorPool);

        // --- Allocate Set 0 (Lights Layout) ---
        VkDescriptorSetAllocateInfo allocLight = {0};
        allocLight.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocLight.descriptorPool = modelDescriptorPool;
        allocLight.descriptorSetCount = 1;
        allocLight.pSetLayouts = &lightDescriptorSetLayout;
        if (vkAllocateDescriptorSets(vk_device, &allocLight, &g_lights.descriptorSet) != VK_SUCCESS) {
            LOGE("FAILED LIGHT SET ALLOCATION");
        }

        // --- Allocate Set 1 (Model / Instance Layout) ---
        VkDescriptorSetAllocateInfo allocModel = {0};
        allocModel.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocModel.descriptorPool = modelDescriptorPool;
        allocModel.descriptorSetCount = 1;
        allocModel.pSetLayouts = &modelDescriptorSetLayout;
        if (vkAllocateDescriptorSets(vk_device, &allocModel, &modelDescriptorSet) != VK_SUCCESS) {
            LOGE("FAILED MODEL SET ALLOCATION");
            exit(1);
        }

        // Initial camera UBO write (Binding 0 on Set 1)
        VkDescriptorBufferInfo camInfo = { cameraUBOBuffer, 0, sizeof(CameraUBO) };
        VkWriteDescriptorSet write = {0};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = modelDescriptorSet;
        write.dstBinding = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount = 1;
        write.pBufferInfo = &camInfo;
        vkUpdateDescriptorSets(vk_device, 1, &write, 0, NULL);
    }

    void update_model_descriptor(void)
    {
        if (g_model_system.instanceBuffer == VK_NULL_HANDLE) return;

        VkDescriptorBufferInfo instInfo = {
            g_model_system.instanceBuffer,
            0,
            g_model_system.instanceBufferSize
        };

        VkWriteDescriptorSet write = {0};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = modelDescriptorSet;
        write.dstBinding = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write.descriptorCount = 1;
        write.pBufferInfo = &instInfo;
        if (modelDescriptorSet == VK_NULL_HANDLE) {
            LOGE("MODEL DESCRIPTOR IS NULL");
            exit(1);
        }
        vkUpdateDescriptorSets(vk_device, 1, &write, 0, NULL);
    }

    void init_model_system(void)
    {
        create_model_descriptors();
        create_model_pipeline();

        // --- 1. ALLOCATE MASTER GEOMETRY BUFFERS FIRST ---
        // These must exist before any asset parsing or binary loading happens!
        g_model_system.masterVertexTailCount = 0;
        g_model_system.masterIndexTailCount = 0;

        create_vulkan_buffer(MAX_SCENE_VERTICES * sizeof(Vertex3D), 
                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                             &g_model_system.masterVertexBuffer, 
                             &g_model_system.masterVertexMemory);

        create_vulkan_buffer(MAX_SCENE_INDICES * sizeof(uint32_t), 
                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
                             &g_model_system.masterIndexBuffer, 
                             &g_model_system.masterIndexMemory);

        // --- MAP ONCE AND HOLD PERSISTENTLY ---
        vkMapMemory(vk_device, g_model_system.masterVertexMemory, 0, VK_WHOLE_SIZE, 0, &g_model_system.masterVertexMappedData);
        vkMapMemory(vk_device, g_model_system.masterIndexMemory,  0, VK_WHOLE_SIZE, 0, &g_model_system.masterIndexMappedData);

        char resolved_bin_path[512];
        if (g_current_project_path && strlen(g_current_project_path) > 0) {
            snprintf(resolved_bin_path, sizeof(resolved_bin_path), "%s/model_state.bin", g_current_project_path);
        } else {
            LOGE("Invalid project path when trying to save model_state.bin");
        }

        // --- 2. Attempt to load the pre-baked spatial binary ---
        if (load_model_state(resolved_bin_path)) {
            LOGI("Successfully initialized world from binary file: %s", resolved_bin_path);
            // --- FIX: Force asset parsing to populate the GPU Master Buffers & model tracking state ---
            find_or_load_model("../../assets/meshes/cs_goddess_statue_opt.glb");
        } 
        else {
            LOGI("No project binary found at %s. Running procedural setup loop to generate assets...", resolved_bin_path);

            // Pre-allocate capacity in memory BEFORE generating data ---
            uint32_t procedural_target_capacity = 40000;
            if (g_model_system.instanceCapacity < procedural_target_capacity) {
                g_model_system.instances = realloc(g_model_system.instances, procedural_target_capacity * sizeof(CPUInstanceData));
                g_model_system.instanceCapacity = procedural_target_capacity;
            }
            g_model_system.instanceCount = 0;

            generate_procedural_template_world(procedural_target_capacity);

            export_project_binary(resolved_bin_path);
            load_project_binary(resolved_bin_path);

            find_or_load_model("../../assets/meshes/cs_goddess_statue_opt.glb");
        }

        // --- 3. Allocate tracking metrics maps with multi-LOD bucket capacity ---
        g_model_system.visibleInstancesCPU = realloc(
            g_model_system.visibleInstancesCPU, 
            g_model_system.instanceCount * sizeof(InstanceData)
        );

        // --- 4. Create instance storage buffers ---
        g_model_system.instanceBufferSize = 0;
        create_instance_buffer(g_model_system.instanceCapacity);
        update_model_descriptor();

        // Run an initial runtime cull invocation now that memory buffers and descriptor bindings are active!
        update_model_streaming_and_culling(0.0f, 0.0f, 0.0f, 500.0f);
    }

    void cleanup_model_system(void)
    {
        // 1. Free structural names and CPU arrays safely
        for (uint32_t i = 0; i < g_model_system.modelCount; ++i) {
            free(g_model_system.models[i].name);
            // Note: No individual VkBuffers to destroy here anymore!
        }

        free(g_model_system.models);
        free(g_model_system.instances);
        free(g_model_system.visibleInstancesCPU);
        
        // Free Option B multi-LOD buckets maps
        free(g_model_system.model_lod_visible_counts);
        free(g_model_system.model_lod_gpu_offsets);

        // Unmap master blocks before dropping handles
        if (g_model_system.masterVertexMappedData != NULL) {
            vkUnmapMemory(vk_device, g_model_system.masterVertexMemory);
        }
        if (g_model_system.masterIndexMappedData != NULL) {
            vkUnmapMemory(vk_device, g_model_system.masterIndexMemory);
        }

        // 2. Clear out master vertex pipeline structures
        if (g_model_system.masterVertexBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(vk_device, g_model_system.masterVertexBuffer, NULL);
            vkFreeMemory(vk_device, g_model_system.masterVertexMemory, NULL);
        }

        // 3. Clear out master index pipeline structures
        if (g_model_system.masterIndexBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(vk_device, g_model_system.masterIndexBuffer, NULL);
            vkFreeMemory(vk_device, g_model_system.masterIndexMemory, NULL);
        }

        // 4. Release global frame streaming structures
        if (g_model_system.instanceBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(vk_device, g_model_system.instanceBuffer, NULL);
            vkFreeMemory(vk_device, g_model_system.instanceMemory, NULL);
        }

        g_model_system = (ModelSystem){0};
    }
#endif