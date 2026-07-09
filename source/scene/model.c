#include "pch.h"
#include "scene/model.h"
ModelSystem g_model_system = {0};
#ifdef __EMSCRIPTEN__
    #define _strdup strdup
    extern WGPUDevice device;
    extern const char* model_vertex_wgsl;
    extern const char* model_fragment_wgsl;
#else
    #define CGLTF_IMPLEMENTATION
    #define CGLTF_MESHOPT_DECODE 1
    #define MESHOPTIMIZER_IMPLEMENTATION
    #include "cgltf.h"
    extern uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
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
        vkCreateShaderModule(vk_device, &sm, NULL, &modelVertModule);
        free(vert_code);

        sm.codeSize = frag_size; sm.pCode = frag_code;
        vkCreateShaderModule(vk_device, &sm, NULL, &modelFragModule);
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
        raster.cullMode = VK_CULL_MODE_BACK_BIT;
        raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo ms = {0};
        ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

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
            printf("Failed to create model pipeline\n");
            exit(1);
        }

        printf("Model pipeline created successfully\n");
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
            printf("Failed to create model pipeline layout\n");
            exit(1);
        }

        VkDescriptorPoolSize poolSizes[2] = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2},  // camera + lights
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}
        };
        VkDescriptorPoolCreateInfo poolInfo = {0};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 2;
        poolInfo.pPoolSizes = poolSizes;
        poolInfo.maxSets = 2;
        vkCreateDescriptorPool(vk_device, &poolInfo, NULL, &modelDescriptorPool);

        VkDescriptorSetAllocateInfo allocInfo = {0};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = modelDescriptorPool;
        allocInfo.descriptorSetCount = 2;
        allocInfo.pSetLayouts = setLayouts;

        VkDescriptorSet sets[2];

        if (vkAllocateDescriptorSets(vk_device, &allocInfo, sets) != VK_SUCCESS) {
            printf("FAILED TO ALLOCATE DESCRIPTOR SETS\n");
            exit(1);
        }

        g_lights.descriptorSet = sets[0];
        modelDescriptorSet = sets[1];

        // Initial camera UBO write
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
            printf("MODEL DESCRIPTOR IS NULL\n");
            exit(1);
        }
        vkUpdateDescriptorSets(vk_device, 1, &write, 0, NULL);
    }
#endif

#ifdef __EMSCRIPTEN__
    WGPURenderPipeline modelPipeline = NULL;
    WGPUPipelineLayout modelPipelineLayout = NULL;
    WGPUBindGroup modelBindGroup = NULL;        // set 0: camera + lights + instances
    WGPUBindGroupLayout    modelBindGroupLayout = NULL;   // ← NEW: we need this

    void create_model_pipeline_webgpu(void)
    {
        WGPUShaderModule modelVertexShader   = create_shader_module(device, model_vertex_wgsl,   "model_vertex");
        WGPUShaderModule modelFragmentShader = create_shader_module(device, model_fragment_wgsl, "model_fragment");

        // Vertex layout (position, normal, uv)
        WGPUVertexAttribute attrs[3] = {
            { .format = WGPUVertexFormat_Float32x3, .offset = offsetof(Vertex3D, x), .shaderLocation = 0 },
            { .format = WGPUVertexFormat_Float32x3, .offset = offsetof(Vertex3D, nx), .shaderLocation = 1 },
            { .format = WGPUVertexFormat_Float32x2, .offset = offsetof(Vertex3D, u), .shaderLocation = 2 }
        };

        WGPUVertexBufferLayout vertexBufferLayout = {
            .arrayStride = sizeof(Vertex3D),
            .stepMode = WGPUVertexStepMode_Vertex,
            .attributeCount = 3,
            .attributes = attrs
        };

        // Instance layout (mat4 + color) - 5 vec4s
        WGPUVertexAttribute instAttrs[5] = {
            { .format = WGPUVertexFormat_Float32x4, .offset = 0,  .shaderLocation = 3 },
            { .format = WGPUVertexFormat_Float32x4, .offset = 16, .shaderLocation = 4 },
            { .format = WGPUVertexFormat_Float32x4, .offset = 32, .shaderLocation = 5 },
            { .format = WGPUVertexFormat_Float32x4, .offset = 48, .shaderLocation = 6 },
            { .format = WGPUVertexFormat_Float32x4, .offset = 64, .shaderLocation = 7 } // color
        };

        WGPUVertexBufferLayout instanceBufferLayout = {
            .arrayStride = sizeof(InstanceData),
            .stepMode = WGPUVertexStepMode_Instance,
            .attributeCount = 5,
            .attributes = instAttrs
        };

        WGPUVertexState vertexState = {
            .module = modelVertexShader,
            .entryPoint = (WGPUStringView){.data = "main", .length = 4},
            .bufferCount = 1, // Only tracking vertexBufferLayout now
            .buffers = (WGPUVertexBufferLayout[]){ vertexBufferLayout }
        };

        WGPUFragmentState fragmentState = {
            .module = modelFragmentShader,
            .entryPoint = (WGPUStringView){.data = "main", .length = 4},
            .targetCount = 1,
            .targets = &(WGPUColorTargetState){
                .format = WGPUTextureFormat_BGRA8Unorm,
                .blend = NULL,
                .writeMask = WGPUColorWriteMask_All
            }
        };

        // Depth state
        WGPUDepthStencilState depthStencil = {0};
        depthStencil.format = WGPUTextureFormat_Depth24Plus;
        depthStencil.depthWriteEnabled = true;
        depthStencil.depthCompare = WGPUCompareFunction_Less;

        WGPURenderPipelineDescriptor pipelineDesc = {
            .label = (WGPUStringView){.data = "ModelPipeline", .length = 13},
            .layout = modelPipelineLayout,
            .vertex = vertexState,
            .primitive = {
                .topology = WGPUPrimitiveTopology_TriangleList,
                .cullMode = WGPUCullMode_None,
                .frontFace = WGPUFrontFace_CCW
            },
            .depthStencil = &depthStencil,
            .multisample = { .count = 1, .mask = 0xFFFFFFFF },
            .fragment = &fragmentState
        };

        modelPipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
        wgpuShaderModuleRelease(modelVertexShader);
        wgpuShaderModuleRelease(modelFragmentShader);
    }

    // Create bind group layout + bind group for models (camera + lights + instances)
    static void create_model_bind_group_layout(void)
    {
        WGPUBindGroupLayoutEntry entries[3] = {
            // Binding 0: Camera UBO
            {
                .binding = 0,
                .visibility = WGPUShaderStage_Vertex,
                .buffer = {
                    .type = WGPUBufferBindingType_Uniform,
                    .minBindingSize = sizeof(CameraUBO)
                }
            },
            // Binding 1: Lighting UBO
            {
                .binding = 1,
                .visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment,
                .buffer = {
                    .type = WGPUBufferBindingType_Uniform,
                    .minBindingSize = sizeof(LightingUBO)
                }
            },
            // Binding 2: Instance SSBO (storage buffer)
            {
                .binding = 2,
                .visibility = WGPUShaderStage_Vertex,
                .buffer = {
                    .type = WGPUBufferBindingType_ReadOnlyStorage,
                    .minBindingSize = 0   // dynamic size is ok for storage buffers
                }
            }
        };

        WGPUBindGroupLayoutDescriptor layoutDesc = {
            .label = (WGPUStringView){.data = "ModelBindGroupLayout", .length = 21},
            .entryCount = 3,
            .entries = entries
        };

        modelBindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &layoutDesc);

        WGPUPipelineLayoutDescriptor pipeLayoutDesc = {
            .label = (WGPUStringView){.data = "ModelPipelineLayout", .length = 19},
            .bindGroupLayoutCount = 1,
            .bindGroupLayouts = &modelBindGroupLayout
        };

        modelPipelineLayout = wgpuDeviceCreatePipelineLayout(device, &pipeLayoutDesc);
    }
#endif

    void init_model_system(void)
    {
    #ifdef __EMSCRIPTEN__
        create_model_bind_group_layout();
        create_model_pipeline_webgpu();
    #else
        create_model_descriptors();
        create_model_pipeline();
    #endif

        g_model_system.models = NULL;
        g_model_system.modelCount = 0;
        g_model_system.modelCapacity = 0;

        g_model_system.instances = NULL;
        g_model_system.instanceCount = 0;
        g_model_system.instanceCapacity = 256;
        g_model_system.instances = malloc(g_model_system.instanceCapacity * sizeof(CPUInstanceData));

    #ifdef __EMSCRIPTEN__
        WGPUBufferDescriptor bufDesc = {
            .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst,
            .size = g_model_system.instanceCapacity * sizeof(InstanceData),
            .mappedAtCreation = false
        };
        g_model_system.instanceBuffer = wgpuDeviceCreateBuffer(device, &bufDesc);
    #else
        g_model_system.instanceBuffer = VK_NULL_HANDLE;
        g_model_system.instanceMemory = VK_NULL_HANDLE;
        g_model_system.instanceBufferSize = 0;

        create_instance_buffer(g_model_system.instanceCapacity);
    #endif

        // --- This now runs on BOTH Desktop and Web ---
        for (int i = 0; i < 200; i++) {
            float transform[16] = {0};
            matrix_identity(transform);

            transform[3]  = (float)(i % 5) * 14.0f - 28.0f;
            transform[7]  = 0.0f;
            transform[11] = (float)(i / 5) * 14.0f - 18.0f;

            float color[4] = {0.2f + (i % 5) * 0.2f, 0.6f, 0.8f, 1.0f};

            // Use standard path layout for web, relative dot-walk for native desktop
    #ifdef __EMSCRIPTEN__
            add_model_instance("assets/meshes/cs_goddess_statue_opt.glb", transform, color);
    #else
            add_model_instance("../../assets/meshes/cs_goddess_statue_opt.glb", transform, color);
    #endif
        }

        update_model_instances();

    #ifndef __EMSCRIPTEN__
        update_model_descriptor();
    #endif
    }

    Model* store_and_free_model(const char* glb_path, Mesh mesh,
                                 Vertex3D* vertices, uint32_t* indices,
                                 float* positions, float* normals, float* uvs,
    #ifndef __EMSCRIPTEN__
                                 cgltf_data* data // Only exists on Native
    #else
                                 void* data // Dummy pointer for Web to keep signature simple
    #endif
    )
    {
        // === Grow the models array if needed ===
        if (g_model_system.modelCount >= g_model_system.modelCapacity) {
            g_model_system.modelCapacity = g_model_system.modelCapacity ? g_model_system.modelCapacity * 2 : 8;
            g_model_system.models = realloc(g_model_system.models,
                                            g_model_system.modelCapacity * sizeof(Model));
        }

        // Store the new Model
        Model* model = &g_model_system.models[g_model_system.modelCount++];
        model->name = _strdup(glb_path);           // store full path for simplicity
        model->mesh = mesh;                       // copy the Mesh struct
        model->instanceOffset = 0;                // we'll improve this later
        model->instanceCount = 0;

        // Calculate bounding sphere from vertex positions
        float min_x = 1e6f, min_y = 1e6f, min_z = 1e6f;
        float max_x = -1e6f, max_y = -1e6f, max_z = -1e6f;
        for (uint32_t v = 0; v < mesh.vertexCount; v++) {
            if (vertices[v].x < min_x) min_x = vertices[v].x;
            if (vertices[v].y < min_y) min_y = vertices[v].y;
            if (vertices[v].z < min_z) min_z = vertices[v].z;
            if (vertices[v].x > max_x) max_x = vertices[v].x;
            if (vertices[v].y > max_y) max_y = vertices[v].y;
            if (vertices[v].z > max_z) max_z = vertices[v].z;
        }
        // Center is mid-point of bounding box
        model->local_center[0] = (min_x + max_x) * 0.5f;
        model->local_center[1] = (min_y + max_y) * 0.5f;
        model->local_center[2] = (min_z + max_z) * 0.5f;

        // Radius is maximum distance from center to any vertex
        float max_r2 = 0.0f;
        for (uint32_t v = 0; v < mesh.vertexCount; v++) {
            float dx = vertices[v].x - model->local_center[0];
            float dy = vertices[v].y - model->local_center[1];
            float dz = vertices[v].z - model->local_center[2];
            float dist2 = dx*dx + dy*dy + dz*dz;
            if (dist2 > max_r2) max_r2 = dist2;
        }
        model->local_radius = sqrtf(max_r2);

        // Cleanup temporary CPU data
        free(vertices);
        free(indices);
        free(positions);
        free(normals);
        free(uvs);
    #ifndef __EMSCRIPTEN__
        cgltf_free(data);
        printf("Loaded model: %s (%u verts, %u indices)\n", 
               glb_path, mesh.vertexCount, mesh.indexCount);
    #endif
        return model;
    }

 #ifdef __EMSCRIPTEN__
    void on_load_success(emscripten_fetch_t *fetch);

    void load_model_from_server(const char* url, Model* model_ptr) {
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, "GET");
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        attr.userData = model_ptr; 
        attr.onsuccess = on_load_success;
        emscripten_fetch(&attr, url);
    }

    void on_load_success(emscripten_fetch_t *fetch) {
        Model* model = (Model*)fetch->userData;
        uint8_t* buffer = (uint8_t*)fetch->data;

        // 1. Basic GLB Header Validation
        uint32_t magic = *(uint32_t*)(buffer + 0);
        if (magic != 0x46546C67) { 
            printf("Invalid GLB magic!\n");
            emscripten_fetch_close(fetch);
            return;
        }

        // 2. Safely Locate Binary Data Chunk
        uint32_t jsonLen = *(uint32_t*)(buffer + 12);
        
        // chunk0 starts at 12. Length is jsonLen. 
        // chunk1 header starts exactly at 12 + 8 + jsonLen.
        uint32_t binChunkHeaderOffset = 12 + 8 + jsonLen;
        
        // Skip the 8-byte BIN chunk header (4 bytes length + 4 bytes type) to get to raw data
        uint8_t* binData = buffer + binChunkHeaderOffset + 8; 

        // 3. Dynamic Structural Target Values
        // For your optimized engine asset, extract these numbers from your asset build 
        // pipelines, or check your desktop logger to see what values cgltf provides!
        uint32_t vertexCount = 3824;  // <-- Replace with your actual asset counts!
        uint32_t index_count = 11472; // <-- Replace with your actual asset counts!

        Mesh mesh = {0};
        mesh.vertexCount = vertexCount;
        mesh.indexCount = index_count;

        // Standard GLTF exporters tightly pack vertices first, then indices right behind them
        void* tempVertexData = binData; 
        void* tempIndexData  = binData + (vertexCount * sizeof(Vertex3D));

        // 4. Create WebGPU GPU buffers
        {
            WGPUBufferDescriptor vdesc = {
                .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
                .size = vertexCount * sizeof(Vertex3D),
                .label = "Vertex Buffer"
            };
            mesh.vertexBuffer = wgpuDeviceCreateBuffer(device, &vdesc);
            wgpuQueueWriteBuffer(queue, mesh.vertexBuffer, 0, tempVertexData, vdesc.size);
        }

        if (index_count > 0) {
            WGPUBufferDescriptor idesc = {
                .usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst,
                .size = index_count * sizeof(uint32_t), // ensure match with index type (uint16/uint32)
                .label = "Index Buffer"
            };
            mesh.indexBuffer = wgpuDeviceCreateBuffer(device, &idesc);
            wgpuQueueWriteBuffer(queue, mesh.indexBuffer, 0, tempIndexData, idesc.size);
        }
        
        // 5. Commit data structures to global registry
        model->mesh = mesh;
        model->isLoaded = true; 
        
        printf("Successfully parsed WebGPU model: %s (%d vertices)\n", model->name, vertexCount);
        emscripten_fetch_close(fetch);
    }

    Model* create_empty_model_entry(const char* path) {
        // Pass NULLs/0s for the pointers since we don't have data yet
        Model* model = store_and_free_model(path, (Mesh){0}, NULL, NULL, NULL, NULL, NULL, NULL);
        return model;
    }
#endif

    Model* find_or_load_model(const char* glb_path)
    {
        // === 1. First, check if we already loaded this model ===
        for (uint32_t i = 0; i < g_model_system.modelCount; ++i) {
            if (strcmp(g_model_system.models[i].name, glb_path) == 0) {
                return &g_model_system.models[i];   // Already loaded → reuse
            }
        }

    #ifdef __EMSCRIPTEN__
        // === 2. WebGPU / Server Path ===
        // We don't use cgltf here. We trigger a fetch to the Render server.
        // Since we can't return the full model yet, we create an entry 
        // and mark it as "loading".
        
        Model* model = create_empty_model_entry(glb_path);
        
        // This is your dedicated function that calls JS/Fetch
        load_model_from_server(glb_path, model); 

        return model; // Or a 'placeholder' model pointer
    #else
        // === 3. Native/Vulkan Path ===
        cgltf_options options = {0};
        cgltf_data* data = NULL;
        cgltf_result result = cgltf_parse_file(&options, glb_path, &data);
        if (result != cgltf_result_success) {
            printf("Failed to parse glb: %s\n", glb_path);
            return NULL;
        }
        result = cgltf_load_buffers(&options, data, glb_path);
        if (result != cgltf_result_success) {
            printf("Failed to load buffers for glb: %s\n", glb_path);
            cgltf_free(data);
            return NULL;
        }
        if (data->meshes_count == 0 || data->meshes[0].primitives_count == 0) {
            printf("No mesh/primitive in glb: %s\n", glb_path);
            cgltf_free(data);
            return NULL;
        }

        cgltf_primitive* prim = &data->meshes[0].primitives[0];

        float* positions = NULL;
        float* normals = NULL;
        float* uvs = NULL;
        cgltf_size pos_count = 0, norm_count = 0, uv_count = 0;

        for (cgltf_size i = 0; i < prim->attributes_count; i++) {
            cgltf_attribute* attr = &prim->attributes[i];
            if (attr->type == cgltf_attribute_type_position) {
                pos_count = cgltf_accessor_unpack_floats(attr->data, NULL, 0);
                positions = (float*)malloc(pos_count * sizeof(float));
                cgltf_accessor_unpack_floats(attr->data, positions, pos_count);
            }
            else if (attr->type == cgltf_attribute_type_normal) {
                norm_count = cgltf_accessor_unpack_floats(attr->data, NULL, 0);
                normals = (float*)malloc(norm_count * sizeof(float));
                cgltf_accessor_unpack_floats(attr->data, normals, norm_count);
            }
            else if (attr->type == cgltf_attribute_type_texcoord) {
                uv_count = cgltf_accessor_unpack_floats(attr->data, NULL, 0);
                uvs = (float*)malloc(uv_count * sizeof(float));
                cgltf_accessor_unpack_floats(attr->data, uvs, uv_count);
            }
        }

        uint32_t* indices = NULL;
        cgltf_size index_count = 0;
        if (prim->indices) {
            index_count = prim->indices->count;
            indices = (uint32_t*)malloc(index_count * sizeof(uint32_t));
            cgltf_accessor_unpack_indices(prim->indices, indices, sizeof(uint32_t), index_count);
        }

        uint32_t vertexCount = (uint32_t)(pos_count / 3);
        Vertex3D* vertices = (Vertex3D*)malloc(vertexCount * sizeof(Vertex3D));

        for (uint32_t i = 0; i < vertexCount; i++) {
            vertices[i].x = positions[i*3 + 0];
            vertices[i].y = positions[i*3 + 1];
            vertices[i].z = positions[i*3 + 2];
            vertices[i].nx = normals ? normals[i*3 + 0] : 0.0f;
            vertices[i].ny = normals ? normals[i*3 + 1] : 0.0f;
            vertices[i].nz = normals ? normals[i*3 + 2] : 1.0f;
            vertices[i].u = uvs ? uvs[i*2 + 0] : 0.0f;
            vertices[i].v = uvs ? uvs[i*2 + 1] : 0.0f;
        }

        // Create buffers for the mesh
        Mesh mesh = {0};
        VkDeviceSize vsize = vertexCount * sizeof(Vertex3D);

        create_vulkan_buffer(vsize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                             &mesh.vertexBuffer, &mesh.vertexMemory);

        void* data_ptr;
        vkMapMemory(vk_device, mesh.vertexMemory, 0, vsize, 0, &data_ptr);
        memcpy(data_ptr, vertices, vsize);
        vkUnmapMemory(vk_device, mesh.vertexMemory);

        if (index_count > 0) {
            VkDeviceSize isize = index_count * sizeof(uint32_t);
            create_vulkan_buffer(isize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                 &mesh.indexBuffer, &mesh.indexMemory);

            vkMapMemory(vk_device, mesh.indexMemory, 0, isize, 0, &data_ptr);
            memcpy(data_ptr, indices, isize);
            vkUnmapMemory(vk_device, mesh.indexMemory);
            mesh.indexCount = (uint32_t)index_count;
        }
        mesh.vertexCount = vertexCount;
        Model* model = store_and_free_model(glb_path, mesh, vertices, indices, positions, normals, uvs, data);
        model->isLoaded = true; 
        return model;
    #endif
    }

    void cleanup_model_system(void)
    {
        for (uint32_t i = 0; i < g_model_system.modelCount; ++i) {
            Mesh* m = &g_model_system.models[i].mesh;
            free(g_model_system.models[i].name);

            #ifdef __EMSCRIPTEN__
                if (m->vertexBuffer) wgpuBufferRelease(m->vertexBuffer);
                if (m->indexBuffer)  wgpuBufferRelease(m->indexBuffer);
            #else
                vkDestroyBuffer(vk_device, m->vertexBuffer, NULL);
                vkFreeMemory(vk_device, m->vertexMemory, NULL);
                if (m->indexCount > 0) {
                    vkDestroyBuffer(vk_device, m->indexBuffer, NULL);
                    vkFreeMemory(vk_device, m->indexMemory, NULL);
                }
            #endif
        }

        free(g_model_system.models);
        free(g_model_system.instances);

    #ifdef __EMSCRIPTEN__
        if (g_model_system.instanceBuffer) wgpuBufferRelease(g_model_system.instanceBuffer);
    #else
        if (g_model_system.instanceBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(vk_device, g_model_system.instanceBuffer, NULL);
            vkFreeMemory(vk_device, g_model_system.instanceMemory, NULL);
        }
    #endif

        // Reset everything
        g_model_system = (ModelSystem){0};
    }