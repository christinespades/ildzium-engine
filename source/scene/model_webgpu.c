#include "pch.h"

#ifdef __EMSCRIPTEN__
    #include "scene/model_webgpu.h"
    ModelSystem g_model_system = {0};

    extern WGPUDevice device;
    extern const char* model_vertex_wgsl;
    extern const char* model_fragment_wgsl;
    WGPURenderPipeline modelPipeline = NULL;
    WGPUPipelineLayout modelPipelineLayout = NULL;
    WGPUBindGroup modelBindGroup = NULL;        // set 0: camera + lights + instances
    WGPUBindGroupLayout modelBindGroupLayout = NULL;

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

    bool load_project_binary(const char* path)
    {
        FILE* file = fopen(path, "rb");
        if (!file) return false;

        uint32_t magic = 0;
        uint32_t version = 0;
        fread(&magic, sizeof(uint32_t), 1, file);
        fread(&version, sizeof(uint32_t), 1, file);

        if (magic != 0x494C445A) { // "ILDZ"
            printf("Invalid engine project file header magic alignment!\n");
            fclose(file);
            return false;
        }

        // 1. Read Model manifests names
        fread(&g_model_system.modelCount, sizeof(uint32_t), 1, file);
        g_model_system.models = malloc(g_model_system.modelCount * sizeof(Model));

        for (uint32_t i = 0; i < g_model_system.modelCount; i++) {
            uint32_t name_len = 0;
            fread(&name_len, sizeof(uint32_t), 1, file);
            
            g_model_system.models[i].name = malloc(name_len + 1);
            fread(g_model_system.models[i].name, sizeof(char), name_len, file);
            g_model_system.models[i].name[name_len] = '\0';

            fread(g_model_system.models[i].local_center, sizeof(float), 3, file);
            fread(&g_model_system.models[i].local_radius, sizeof(float), 1, file);
            
            g_model_system.models[i].mesh.vertexBuffer = VK_NULL_HANDLE;
            g_model_system.models[i].mesh.indexBuffer = VK_NULL_HANDLE;
            g_model_system.models[i].mesh.vertexCount = 0;
            g_model_system.models[i].mesh.indexCount = 0;
            g_model_system.models[i].isLoaded = false; 
        }

        // 2. Read Spatial Chunk bounding layout
        fread(&g_model_system.chunkCount, sizeof(uint32_t), 1, file);
        g_model_system.chunks = malloc(g_model_system.chunkCount * sizeof(SpatialChunk));

        uint32_t total_instances_count = 0;
        for (uint32_t i = 0; i < g_model_system.chunkCount; i++) {
            fread(g_model_system.chunks[i].center, sizeof(float), 3, file);
            fread(&g_model_system.chunks[i].radius, sizeof(float), 1, file);
            fread(&g_model_system.chunks[i].instanceOffset, sizeof(uint32_t), 1, file);
            fread(&g_model_system.chunks[i].instanceCount, sizeof(uint32_t), 1, file);
            g_model_system.chunks[i].isLoaded = true;

            total_instances_count += g_model_system.chunks[i].instanceCount;
        }

        // 3. Read Instance details blocks cleanly in linear block allocation chunks
        if (total_instances_count > g_model_system.instanceCapacity) {
            g_model_system.instanceCapacity = total_instances_count;
            g_model_system.instances = realloc(g_model_system.instances, g_model_system.instanceCapacity * sizeof(CPUInstanceData));
            g_model_system.visibleInstancesCPU = realloc(g_model_system.visibleInstancesCPU, g_model_system.instanceCapacity * sizeof(InstanceData));
        }
        
        g_model_system.instanceCount = total_instances_count;
        fread(g_model_system.instances, sizeof(CPUInstanceData), g_model_system.instanceCount, file);

        fclose(file);
        printf("Successfully initialized %u spatial chunks tracking %u instances from binary.\n", 
                g_model_system.chunkCount, g_model_system.instanceCount);
        return true;
    }

    void init_model_system(void)
    {
        create_model_bind_group_layout();
        create_model_pipeline_webgpu();
        const char* project_bin_path = "_project_world.bin";
        if (!load_model_state(project_bin_path)) {
            printf("Web target requires remote fetching of '%s' stream layout.\n", project_bin_path);
            // Fallback or asynchronous range fetch initiation goes here
        }

        WGPUBufferDescriptor bufDesc = {
            .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst,
            .size = g_model_system.instanceCapacity * sizeof(InstanceData),
            .mappedAtCreation = false
        };
        g_model_system.instanceBuffer = wgpuDeviceCreateBuffer(device, &bufDesc);

        // --- This now runs on BOTH Desktop and Web ---
        for (int i = 0; i < 2000; i++) {
            float transform[16] = {0};
            matrix_identity(transform);

            transform[3]  = (float)(i % 5) * 14.0f - 28.0f;
            transform[7]  = 0.0f;
            transform[11] = (float)(i / 5) * 14.0f - 18.0f;

            float color[4] = {0.2f + (i % 5) * 0.2f, 0.6f, 0.8f, 1.0f};

            // Use standard path layout for web, relative dot-walk for native desktop
            add_model_instance("assets/meshes/cs_goddess_statue_opt.glb", transform, color);
        }

        update_model_instances();
    }

    Model* store_and_free_model(
        Model* model,
        const char* glb_path,
        Mesh mesh,
        Vertex3D* vertices,
        uint32_t* indices,
        float* positions,
        float* normals,
        float* uvs,
        void* data
    )
    {
        // Allocate a new entry only if we weren't given one.
        if (!model) {
            if (g_model_system.modelCount >= g_model_system.modelCapacity) {
                g_model_system.modelCapacity =
                    g_model_system.modelCapacity ? g_model_system.modelCapacity * 2 : 8;

                g_model_system.models = realloc(
                    g_model_system.models,
                    g_model_system.modelCapacity * sizeof(Model));
            }

            model = &g_model_system.models[g_model_system.modelCount++];
            memset(model, 0, sizeof(*model));
            model->name = _strdup(glb_path);
        }

        model->mesh = mesh;
        model->instanceOffset = 0;
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
        return model;
    }

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

    Model* find_or_load_model(const char* glb_path)
    {
        Model* existing = NULL;

        // === 1. First, check if we already loaded this model ===
        for (uint32_t i = 0; i < g_model_system.modelCount; ++i) {
            if (strcmp(g_model_system.models[i].name, glb_path) == 0) {
                existing = &g_model_system.models[i];
                if (existing->isLoaded)
                    return existing;
                break;
            }
        }
        // === WebGPU / Server Path ===
        // We don't use cgltf here. We trigger a fetch to the Render server.
        // Since we can't return the full model yet, we create an entry 
        // and mark it as "loading".
        
        Model* model = create_empty_model_entry(glb_path);
        
        // This is your dedicated function that calls JS/Fetch
        load_model_from_server(glb_path, model); 

        return model; // Or a 'placeholder' model pointer
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
        free(g_model_system.visibleInstancesCPU);

        if (g_model_system.instanceBuffer) wgpuBufferRelease(g_model_system.instanceBuffer);
        g_model_system = (ModelSystem){0};
    }

    void update_spatial_streaming(float cam_x, float cam_y, float cam_z, float stream_radius)
    {
        static int frame = 0;
        frame++;
        if (frame % 10 == 0) {
            int loaded = 0;
            for (uint32_t i = 0; i < g_model_system.modelCount; i++) {
                if (g_model_system.models[i].isLoaded) loaded++;
            }
            printf("Frame %d | Models: %d/%d loaded | Visible: %u | Culled: %u | Total instances: %u\n",
                   frame, loaded, g_model_system.modelCount,
                   g_model_system.visibleCount, g_culled_count, g_model_system.instanceCount);
        }

        g_model_system.visibleCount = 0;
        g_culled_count = g_model_system.instanceCount; // Reset culling statistics tracking

        // Clear global visibility counters for the render pass loop
        for (uint32_t i = 0; i < g_model_system.modelCount; i++) {
            g_model_visible_counts[i] = 0;
            // If your architecture sets offsets linearly, we'll assign them on the fly below
        }

        for (uint32_t i = 0; i < g_model_system.chunkCount; i++) {
            SpatialChunk* chunk = &g_model_system.chunks[i];

            float dx = chunk->center[0] - cam_x;
            float dy = chunk->center[1] - cam_y;
            float dz = chunk->center[2] - cam_z;
            float distance = sqrtf(dx * dx + dy * dy + dz * dz) - chunk->radius;

            if (distance <= stream_radius) {
                for (uint32_t j = 0; j < chunk->instanceCount; j++) {
                    uint32_t global_idx = chunk->instanceOffset + j;
                    CPUInstanceData* cpu_src = &g_model_system.instances[global_idx];

                    Model* model = &g_model_system.models[cpu_src->model_index];
                    
                    if (!model->isLoaded) {
                        find_or_load_model(model->name);
                    }

                    if (model->isLoaded) {
                        // 1. Build the continuous staging array for the GPU buffer transfer
                        InstanceData* gpu_dest = &g_model_system.visibleInstancesCPU[g_model_system.visibleCount];
                        memcpy(gpu_dest->model, cpu_src->model, sizeof(float) * 16);
                        memcpy(gpu_dest->color, cpu_src->color, sizeof(float) * 4);

                        // 2. Set up the tracking offset for the shader's gl_InstanceIndex fallback.
                        // This links the instance position in the GPU array to the correct model bucket.
                        if (g_model_visible_counts[cpu_src->model_index] == 0) {
                            g_model_gpu_offsets[cpu_src->model_index] = g_model_system.visibleCount;
                        }

                        g_model_visible_counts[cpu_src->model_index]++;
                        g_model_system.visibleCount++;
                        g_culled_count--; // Mark instance as visible
                    }
                }
            }
        }

        // Push the tightly packed visible instances array right to the GPU
        if (g_model_system.visibleCount > 0) {
            wgpuQueueWriteBuffer(queue, g_model_system.instanceBuffer, 0,
                                 g_model_system.visibleInstancesCPU,
                                 g_model_system.visibleCount * sizeof(InstanceData));
        }
    }

    #define MAX_CHUNKS_X 16
    #define MAX_CHUNKS_Z 16

    void export_project_binary(const char* output_path)
    {
        FILE* file = fopen(output_path, "wb");
        if (!file) {
            printf("Failed to open file for binary export: %s\n", output_path);
            return;
        }

        // 1. Write Header Magic & Metadata
        uint32_t magic = 0x494C445A; // "ILDZ"
        uint32_t version = 1;
        fwrite(&magic, sizeof(uint32_t), 1, file);
        fwrite(&version, sizeof(uint32_t), 1, file);
        
        // Reserve space for offsets we'll write later
        uint32_t model_count = g_model_system.modelCount;
        fwrite(&model_count, sizeof(uint32_t), 1, file);
        
        // 2. Export Unique Model/Mesh Resource Manifest Names
        for (uint32_t i = 0; i < model_count; i++) {
            uint32_t name_len = (uint32_t)strlen(g_model_system.models[i].name);
            fwrite(&name_len, sizeof(uint32_t), 1, file);
            fwrite(g_model_system.models[i].name, sizeof(char), name_len, file);
            
            // Save intrinsic bounds
            fwrite(g_model_system.models[i].local_center, sizeof(float), 3, file);
            fwrite(&g_model_system.models[i].local_radius, sizeof(float), 1, file);
        }

        // 3. Spatial Sorting Phase: Group instances into dynamic Grid Chunks
        // Find absolute world dimensions of instances to bound the chunks
        float min_x = 1e6f, max_x = -1e6f, min_z = 1e6f, max_z = -1e6f;
        for (uint32_t i = 0; i < g_model_system.instanceCount; i++) {
            float x = g_model_system.instances[i].bounding_center[0];
            float z = g_model_system.instances[i].bounding_center[2];
            if (x < min_x) min_x = x; if (x > max_x) max_x = x;
            if (z < min_z) min_z = z; if (z > max_z) max_z = z;
        }
        
        float size_x = (max_x - min_x) + 0.1f;
        float size_z = (max_z - min_z) + 0.1f;

        // Temporary allocation bucket array for the 2D grid sorting
        uint32_t total_buckets = MAX_CHUNKS_X * MAX_CHUNKS_Z;
        uint32_t* bucket_counts = calloc(total_buckets, sizeof(uint32_t));
        CPUInstanceData** buckets = calloc(total_buckets, sizeof(CPUInstanceData*));
        uint32_t* bucket_capacities = calloc(total_buckets, sizeof(uint32_t));

        for (uint32_t i = 0; i < g_model_system.instanceCount; i++) {
            CPUInstanceData inst = g_model_system.instances[i];
            int ix = (int)(((inst.bounding_center[0] - min_x) / size_x) * MAX_CHUNKS_X);
            int  iz = (int)(((inst.bounding_center[2] - min_z) / size_z) * MAX_CHUNKS_Z);
            if (ix < 0) ix = 0; if (ix >= MAX_CHUNKS_X) ix = MAX_CHUNKS_X - 1;
            if (iz < 0) iz = 0; if (iz >= MAX_CHUNKS_Z) iz = MAX_CHUNKS_Z - 1;

            uint32_t b_idx = iz * MAX_CHUNKS_X + ix;
            if (bucket_counts[b_idx] >= bucket_capacities[b_idx]) {
                bucket_capacities[b_idx] = bucket_capacities[b_idx] ? bucket_capacities[b_idx] * 2 : 16;
                buckets[b_idx] = realloc(buckets[b_idx], bucket_capacities[b_idx] * sizeof(CPUInstanceData));
            }
            buckets[b_idx][bucket_counts[b_idx]++] = inst;
        }

        // Count how many chunks actually contain valid instance groups
        uint32_t valid_chunks = 0;
        for (uint32_t i = 0; i < total_buckets; i++) {
            if (bucket_counts[i] > 0) valid_chunks++;
        }
        fwrite(&valid_chunks, sizeof(uint32_t), 1, file);

        // 4. Output Spatial Chunk Metadata blocks followed by raw instances stream
        uint32_t running_offset = 0;
        for (uint32_t i = 0; i < total_buckets; i++) {
            if (bucket_counts[i] == 0) continue;

            // Calculate a bounding sphere enclosing all instances in this grid bucket
            float c_min_x = 1e6f, c_max_x = -1e6f, c_min_y = 1e6f, c_max_y = -1e6f, c_min_z = 1e6f, c_max_z = -1e6f;
            for (uint32_t j = 0; j < bucket_counts[i]; j++) {
                float x = buckets[i][j].bounding_center[0];
                float y = buckets[i][j].bounding_center[1];
                float z = buckets[i][j].bounding_center[2];
                float r = buckets[i][j].bounding_radius;
                if (x - r < c_min_x) c_min_x = x - r; if (x + r > c_max_x) c_max_x = x + r;
                if (y - r < c_min_y) c_min_y = y - r; if (y + r > c_max_y) c_max_y = y + r;
                if (z - r < c_min_z) c_min_z = z - r; if (z + r > c_max_z) c_max_z = z + r;
            }

            SpatialChunk chunkDesc = {0};
            chunkDesc.center[0] = (c_min_x + c_max_x) * 0.5f;
            chunkDesc.center[1] = (c_min_y + c_max_y) * 0.5f;
            chunkDesc.center[2] = (c_min_z + c_max_z) * 0.5f;
            
            float dx = c_max_x - chunkDesc.center[0];
            float dy = c_max_y - chunkDesc.center[1];
            float dz = c_max_z - chunkDesc.center[2];
            chunkDesc.radius = sqrtf(dx*dx + dy*dy + dz*dz);
            chunkDesc.instanceOffset = running_offset;
            chunkDesc.instanceCount = bucket_counts[i];

            // Write chunk description header block
            fwrite(&chunkDesc.center, sizeof(float), 3, file);
            fwrite(&chunkDesc.radius, sizeof(float), 1, file);
            fwrite(&chunkDesc.instanceOffset, sizeof(uint32_t), 1, file);
            fwrite(&chunkDesc.instanceCount, sizeof(uint32_t), 1, file);

            running_offset += bucket_counts[i];
        }

        // 5. Append Instance Data Blocks linearly behind headers
        for (uint32_t i = 0; i < total_buckets; i++) {
            if (bucket_counts[i] == 0) continue;
            fwrite(buckets[i], sizeof(CPUInstanceData), bucket_counts[i], file);
            free(buckets[i]);
        }

        free(bucket_counts);
        free(buckets);
        free(bucket_capacities);
        fclose(file);
        printf("Successfully baked world spatial index profile!\n");
    }

    extern WGPUDevice device;
    extern WGPUQueue queue;

    void on_chunk_binary_received(emscripten_fetch_t *fetch) 
    {
        // Retrieve the index of the chunk we requested via userData payload
        uint32_t chunk_index = (uint32_t)(uintptr_t)fetch->userData;
        
        if (fetch->status != 200) {
            printf("Failed to stream spatial chunk %u! HTTP status: %d\n", chunk_index, fetch->status);
            g_model_system.chunks[chunk_index].isLoaded = false;
            emscripten_fetch_close(fetch);
            return;
        }

        SpatialChunk* chunk = &g_model_system.chunks[chunk_index];
        uint8_t* raw_bytes = (uint8_t*)fetch->data;
        
        // 1. Read counts directly out of the binary payload header layout
        uint32_t vertex_count = *(uint32_t*)(raw_bytes + 0);
        uint32_t index_count  = *(uint32_t*)(raw_bytes + 4);
        
        // 2. Compute tight pointer strides 
        Vertex3D* vertex_data = (Vertex3D*)(raw_bytes + 8);
        uint32_t* index_data  = (uint32_t*)(raw_bytes + 8 + (vertex_count * sizeof(Vertex3D)));

        // We look up or create a mesh reference for this chunk
        // (If each chunk owns its specific GPU submesh)
        Mesh mesh = {0};
        mesh.vertexCount = vertex_count;
        mesh.indexCount = index_count;

        // 3. Create and commit the WebGPU Vertex Buffer
        if (vertex_count > 0) {
            WGPUBufferDescriptor vDesc = {
                .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
                .size = vertex_count * sizeof(Vertex3D),
                .label = (WGPUStringView){.data = "Streamed Chunk Vertex Buffer", .length = 29},
                .mappedAtCreation = false
            };
            mesh.vertexBuffer = wgpuDeviceCreateBuffer(device, &vDesc);
            wgpuQueueWriteBuffer(queue, mesh.vertexBuffer, 0, vertex_data, vDesc.size);
        }

        // 4. Create and commit the WebGPU Index Buffer
        if (index_count > 0) {
            WGPUBufferDescriptor iDesc = {
                .usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst,
                .size = index_count * sizeof(uint32_t),
                .label = (WGPUStringView){.data = "Streamed Chunk Index Buffer", .length = 28},
                .mappedAtCreation = false
            };
            mesh.indexBuffer = wgpuDeviceCreateBuffer(device, &iDesc);
            wgpuQueueWriteBuffer(queue, mesh.indexBuffer, 0, index_data, iDesc.size);
        }

        // 5. Update state so our frame render loop registers it immediately
        // If your chunks map directly to base model IDs, apply this to the model registry:
        if (chunk_index < g_model_system.modelCount) {
            g_model_system.models[chunk_index].mesh = mesh;
            g_model_system.models[chunk_index].isLoaded = true;
        }

        chunk->isLoaded = true;
        
        printf("Successfully extracted streaming asset block %u (%u verts, %u indices)\n", 
               chunk_index, vertex_count, index_count);
               
        emscripten_fetch_close(fetch);
    }
#endif