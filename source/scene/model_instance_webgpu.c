#include "pch.h"

#ifdef __EMSCRIPTEN__
    #include "scene/model_instance_webgpu.h"

    extern WGPUDevice device;

    void add_model_instance(const char* model_name, const float transform[16], const float color[4])
    {
        static Model* cached_model = NULL;
        static const char* last_name = NULL;

        if (cached_model == NULL || strcmp(model_name, last_name) != 0) {
            cached_model = find_or_load_model(model_name);
            last_name = model_name; 
        }

        if (!cached_model) return;

        // Find the actual index of the cached model within our system
        uint32_t current_model_idx = 0;
        for (uint32_t m = 0; m < g_model_system.modelCount; m++) {
            if (&g_model_system.models[m] == cached_model) {
                current_model_idx = m;
                break;
            }
        }

        if (g_model_system.instanceCount >= g_model_system.instanceCapacity) {
            g_model_system.instanceCapacity *= 2;
            if (g_model_system.instanceCapacity == 0) g_model_system.instanceCapacity = 256;
            g_model_system.instances = realloc(g_model_system.instances,
                                               g_model_system.instanceCapacity * sizeof(CPUInstanceData)); // <-- Use CPU size
        }

        uint32_t idx = g_model_system.instanceCount++;
        CPUInstanceData* inst = &g_model_system.instances[idx]; // <-- Use CPU pointer type

        memcpy(inst->model, transform, 16 * sizeof(float));
        memcpy(inst->color, color, 4 * sizeof(float));
        
        inst->model_index = current_model_idx;
        inst->bounding_center[0] = cached_model->local_center[0] + transform[3];
        inst->bounding_center[1] = cached_model->local_center[1] + transform[7];
        inst->bounding_center[2] = cached_model->local_center[2] + transform[11];
        inst->bounding_radius = cached_model->local_radius; 

        cached_model->instanceCount++;
    }

    // Track how many instances of each model survived culling this frame
    uint32_t* g_model_visible_counts = NULL;
    uint32_t* g_model_gpu_offsets = NULL;

    void update_model_instances(void)
    {
        if (g_model_system.instanceCount == 0) return;

        // Reset Engine Counters
        g_drawn_count = 0;
        g_culled_count = 0;

        // Prepare internal bookkeeping arrays
        g_model_visible_counts = realloc(g_model_visible_counts, g_model_system.modelCount * sizeof(uint32_t));
        g_model_gpu_offsets = realloc(g_model_gpu_offsets, g_model_system.modelCount * sizeof(uint32_t));
        memset(g_model_visible_counts, 0, g_model_system.modelCount * sizeof(uint32_t));

        // 1. First Pass: Culling and calculation of offsets
        uint32_t total_visible = 0;
        for (uint32_t i = 0; i < g_model_system.instanceCount; ++i) {
            CPUInstanceData* cpuInst = &g_model_system.instances[i]; // <-- Read from CPU type
            
            if (is_visible_clip_space(cpuInst->bounding_center, cpuInst->bounding_radius)) {
                g_model_visible_counts[cpuInst->model_index]++;
                total_visible++;
            } else {
                g_culled_count++;
            }
        }

        if (total_visible == 0) return;

        // Calculate packing offsets for continuous writing inside a single GPU storage buffer
        uint32_t current_offset = 0;
        for (uint32_t m = 0; m < g_model_system.modelCount; m++) {
            g_model_gpu_offsets[m] = current_offset;
            current_offset += g_model_visible_counts[m];
        }

        // 2. Second Pass: Write transposed matrices linearly matching the offset map
        // This is the absolute worst-case scenario size based on our tracking footprint
        VkDeviceSize max_capacity_bytes = (VkDeviceSize)g_model_system.instanceCapacity * sizeof(InstanceData);

        if (total_visible == 0) return;
        InstanceData* gpuData = (InstanceData*)malloc(visible_bytes_size);

        uint32_t* write_accumulators = calloc(g_model_system.modelCount, sizeof(uint32_t));

        for (uint32_t i = 0; i < g_model_system.instanceCount; ++i) {
            CPUInstanceData* cpuInst = &g_model_system.instances[i];
            
            if (!is_visible_clip_space(cpuInst->bounding_center, cpuInst->bounding_radius)) {
                continue;
            }

            uint32_t m_idx = cpuInst->model_index;
            uint32_t target_gpu_index = g_model_gpu_offsets[m_idx] + write_accumulators[m_idx];
            write_accumulators[m_idx]++;

            for (int col = 0; col < 4; ++col) {
                for (int row = 0; row < 4; ++row) {
                    gpuData[target_gpu_index].model[col * 4 + row] = cpuInst->model[row * 4 + col];
                }
            }
            memcpy(gpuData[target_gpu_index].color, cpuInst->color, 4 * sizeof(float));
        }

        free(write_accumulators);

        // WebGPU simply updates a precise sub-slice portion of the global instanceBuffer asset
        wgpuQueueWriteBuffer(queue, g_model_system.instanceBuffer, 0, gpuData, visible_bytes_size);
        free(gpuData);
    }
#endif