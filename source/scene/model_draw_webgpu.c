#ifdef __EMSCRIPTEN__
    #include "pch.h"
    #include "scene/model_draw_webgpu.h"
    extern WGPUDevice device;
    extern WGPURenderPipeline modelPipeline;
        extern WGPUBindGroupLayout    modelBindGroupLayout;
    WGPUBindGroup draw_models_webgpu(WGPURenderPassEncoder pass)
    {
        if (g_model_system.modelCount == 0 || g_model_system.instanceCount == 0) return NULL;

        wgpuRenderPassEncoderSetPipeline(pass, modelPipeline);

        // Create bind group using the full allocated buffer size 
        // (or matching your capacity to prevent bind range validation errors)
        WGPUBindGroupEntry entries[3] = {
            { .binding = 0, .buffer = cameraBuffer,   .offset = 0, .size = sizeof(CameraUBO) },
            { .binding = 1, .buffer = lightingBuffer, .offset = 0, .size = sizeof(LightingUBO) },
            { .binding = 2, .buffer = g_model_system.instanceBuffer,
              .offset = 0, .size = (uint64_t)g_model_system.instanceCapacity * sizeof(InstanceData) }
        };

        WGPUBindGroupDescriptor bgDesc = {
            .layout = modelBindGroupLayout,
            .entryCount = 3,
            .entries = entries
        };

        WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(device, &bgDesc);
        wgpuRenderPassEncoderSetBindGroup(pass, 0, bindGroup, 0, NULL);

        // Loop through ALL models exactly like the Vulkan path
        for (uint32_t m_idx = 0; m_idx < g_model_system.modelCount; m_idx++) {
            uint32_t active_instances = g_model_lod_visible_counts[m_idx];
            if (active_instances == 0) continue; 

            Model* m = &g_model_system.models[m_idx];
            if (m->mesh.vertexBuffer == NULL) continue;

            wgpuRenderPassEncoderSetVertexBuffer(pass, 0, m->mesh.vertexBuffer, 0, WGPU_WHOLE_SIZE);

            // Calculate the base instance offset in the storage buffer
            uint32_t base_instance = g_model_gpu_offsets[m_idx];

            if (m->mesh.indexCount > 0) {
                wgpuRenderPassEncoderSetIndexBuffer(pass, m->mesh.indexBuffer,
                                                    WGPUIndexFormat_Uint32, 0, WGPU_WHOLE_SIZE);
                // Pass base_instance to the 5th parameter (firstInstance)
                wgpuRenderPassEncoderDrawIndexed(pass, m->mesh.indexCount,
                                                 active_instances, 0, 0, base_instance);
            } else {
                // Pass base_instance to the 4th parameter (firstInstance)
                wgpuRenderPassEncoderDraw(pass, m->mesh.vertexCount,
                                          active_instances, 0, base_instance);
            }
        }

        return bindGroup;
    }
#endif