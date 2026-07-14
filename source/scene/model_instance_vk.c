#include "pch.h"

#ifndef __EMSCRIPTEN__
    #include "scene/model_instance_vk.h"

    void create_instance_buffer(uint32_t capacity)
    {
        VkDeviceSize newSize = (VkDeviceSize)capacity * sizeof(InstanceData);

        if (g_model_system.instanceBuffer != VK_NULL_HANDLE) {
            // If the old memory was persistently mapped, freeing it automatically unmaps it,
            // but let's clear our pointer tracking state just to be completely safe
            g_model_system.mappedData = NULL; 
            vkDestroyBuffer(vk_device, g_model_system.instanceBuffer, NULL);
            vkFreeMemory(vk_device, g_model_system.instanceMemory, NULL);
        }

        create_vulkan_buffer(newSize,
                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                             &g_model_system.instanceBuffer,
                             &g_model_system.instanceMemory);

        g_model_system.instanceBufferSize = newSize;
        g_model_system.instanceCapacity = capacity;
        
        // Check if create_vulkan_buffer already populated mappedData or if we need to do it
        if (g_model_system.mappedData == NULL) {
            VkResult res = vkMapMemory(vk_device, g_model_system.instanceMemory, 0, g_model_system.instanceBufferSize, 0, &g_model_system.mappedData);
            if (res != VK_SUCCESS) {
                LOGE("Persistent map failed inside create_instance_buffer: %d", res);
            }
        }
    }

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

        g_drawn_count = 0;
        g_culled_count = 0;

        g_model_visible_counts = realloc(g_model_visible_counts, g_model_system.modelCount * sizeof(uint32_t));
        g_model_gpu_offsets = realloc(g_model_gpu_offsets, g_model_system.modelCount * sizeof(uint32_t));
        
        memset(g_model_visible_counts, 0, g_model_system.modelCount * sizeof(uint32_t));

        // --- PASS 1: Calculate accurate model counts using REAL frustum culling ---
        uint32_t total_visible = 0;
        for (uint32_t i = 0; i < g_model_system.instanceCount; ++i) {
            CPUInstanceData* cpuInst = &g_model_system.instances[i];
            
            // Use the exact same visibility test as the writing pass
            int visible = is_visible_clip_space(cpuInst->bounding_center, cpuInst->bounding_radius);
            
            if (visible) {
                g_model_visible_counts[cpuInst->model_index]++;
                total_visible++;
            } else {
                g_culled_count++;
            }
        }

        LOGI("Update Trace 1: Pass 1 Finished. total_visible = %d, culled = %d", total_visible, g_culled_count);

        if (total_visible == 0) {
            // Correctly update system-wide visibility metrics before exiting
            g_model_system.visibleCount = 0;
            LOGI("Update Trace Exit: Bailing out because total_visible is 0!");
            return;
        }
        
        g_model_system.visibleCount = total_visible;

        // --- PASS 2: Compute accurate contiguous offsets ---
        uint32_t current_offset = 0;
        for (uint32_t m = 0; m < g_model_system.modelCount; m++) {
            g_model_gpu_offsets[m] = current_offset;
            current_offset += g_model_visible_counts[m];
        }

        // --- Buffer Management & Resizing ---
        VkDeviceSize max_capacity_bytes = (VkDeviceSize)g_model_system.instanceCapacity * sizeof(InstanceData);
        if (g_model_system.instanceBuffer == VK_NULL_HANDLE || max_capacity_bytes > g_model_system.instanceBufferSize) {
            LOGI("Update Trace 3: Recreating instance buffer!");
            if (g_model_system.instanceBuffer != VK_NULL_HANDLE) {
                vkDeviceWaitIdle(vk_device);
            }
            uint32_t allocation_target_count = g_model_system.instanceCapacity * 2;
            if (allocation_target_count < 256) allocation_target_count = 256;

            create_instance_buffer(allocation_target_count);
            update_model_descriptor();
        }

        // --- PASS 3: Stream matched instances directly into their GPU offsets ---
        InstanceData* gpuData = (InstanceData*)g_model_system.mappedData;
        uint32_t* write_accumulators = calloc(g_model_system.modelCount, sizeof(uint32_t));

        for (uint32_t i = 0; i < g_model_system.instanceCount; ++i) {
            CPUInstanceData* cpuInst = &g_model_system.instances[i];
            
            // Re-evaluate matching Pass 1 parameters exactly
            if (!is_visible_clip_space(cpuInst->bounding_center, cpuInst->bounding_radius)) {
                continue;
            }

            uint32_t m_idx = cpuInst->model_index;
            uint32_t target_gpu_index = g_model_gpu_offsets[m_idx] + write_accumulators[m_idx];
            write_accumulators[m_idx]++;

            // Column-major transposition logic
            for (int col = 0; col < 4; ++col) {
                for (int row = 0; row < 4; ++row) {
                    gpuData[target_gpu_index].model[col * 4 + row] = cpuInst->model[row * 4 + col];
                }
            }
            memcpy(gpuData[target_gpu_index].color, cpuInst->color, 4 * sizeof(float));
        }

        // --- FIX: Spec-compliant memory flushing via VK_WHOLE_SIZE ---
        VkMappedMemoryRange flushRange = {0};
        flushRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        flushRange.memory = g_model_system.instanceMemory;
        flushRange.offset = 0;
        flushRange.size = VK_WHOLE_SIZE; // Tells the driver to safely sweep to the end of the allocation

        vkFlushMappedMemoryRanges(vk_device, 1, &flushRange);
        
        free(write_accumulators);

        LOGI("Update Complete: Total Visible: %d | Culled Count: %d | System Instance Count: %d", 
             total_visible, g_culled_count, g_model_system.instanceCount);
    }
#endif