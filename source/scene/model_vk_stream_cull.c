#include "pch.h"
#include "model_vk_stream_cull.h"

void update_model_streaming_and_culling(float cam_x, float cam_y, float cam_z, float stream_radius)
{
    if (g_model_system.instanceCount == 0) return;

    g_model_system.visibleCount = 0;
    g_stats_distance_culled = 0;
    g_stats_frustum_culled = 0;

    // --- FIX 1: Clear tracking arrays using the struct context pointer ---
    memset(g_model_system.model_lod_visible_counts, 0, g_model_system.modelCount * MAX_LOD_LEVELS * sizeof(uint32_t));

    // =========================================================================
    // --- PASS 1: Identify, evaluate LOD, and count visible objects ---
    // =========================================================================
    for (uint32_t i = 0; i < g_model_system.chunkCount; i++) {
        SpatialChunk* chunk = &g_model_system.chunks[i];

        float dx = chunk->center[0] - cam_x;
        float dy = chunk->center[1] - cam_y;
        float dz = chunk->center[2] - cam_z;
        float distance = sqrtf(dx * dx + dy * dy + dz * dz) - chunk->radius;

        bool pass_distance = !g_enable_distance_culling || (distance <= stream_radius);

        if (pass_distance) {
            for (uint32_t j = 0; j < chunk->instanceCount; j++) {
                uint32_t global_idx = chunk->instanceOffset + j;
                CPUInstanceData* cpu_src = &g_model_system.instances[global_idx];
                
                bool pass_frustum = !g_enable_frustum_culling || 
                                    is_visible_clip_space(cpu_src->bounding_center, cpu_src->bounding_radius);

                if (pass_frustum) {
                    // --- SAFETY CHECK 1: Ensure index is inside model array limits ---
                    if (cpu_src->model_index >= g_model_system.modelCount) {
                        LOGE("Culling Error: Instance has invalid model_index %u (Max is %u)!", 
                             cpu_src->model_index, g_model_system.modelCount);
                        continue; // Skip corrupt instance
                    }

                    Model* model = &g_model_system.models[cpu_src->model_index];
                    
                    // Select LOD Level
                    uint32_t lod_level = 0;
                    if (distance > get_param_float(PARAM_RENDER_LOD3_DISTANCE))      lod_level = 3;
                    else if (distance > get_param_float(PARAM_RENDER_LOD2_DISTANCE)) lod_level = 2;
                    else if (distance > get_param_float(PARAM_RENDER_LOD1_DISTANCE)) lod_level = 1;
                    
                    // --- SAFETY CHECK 2: Prevent unsigned underflow if lod_count is 0 ---
                    if (model->lod_count == 0) {
                        lod_level = 0; 
                    } else if (lod_level >= model->lod_count) {
                        lod_level = model->lod_count - 1;
                    }

                    uint32_t slot = LOD_SLOT(cpu_src->model_index, lod_level);
                    uint32_t max_allowed_slots = g_model_system.modelCount * MAX_LOD_LEVELS;

                    // --- SAFETY CHECK 3: Final out-of-bounds guard ---
                    if (slot >= max_allowed_slots) {
                        LOGE("Culling Error: Calculated slot %u is out of bounds (Max %u)! ModelIdx: %u, LOD: %u", 
                             slot, max_allowed_slots, cpu_src->model_index, lod_level);
                        continue;
                    }

                    // --- FIX 2: Access struct tracker ---
                    g_model_system.model_lod_visible_counts[slot]++;
                } else {
                    g_stats_frustum_culled++;
                }
            }
        } else {
            g_stats_distance_culled += chunk->instanceCount;
        }
    }

    /* debugging
    LOGI("Global modelCount tracked: %u", g_model_system.modelCount);
    for (uint32_t m = 0; m < g_model_system.modelCount; m++) {
        LOGI("Model [%u] Name: '%s' | isLoaded: %s | LOD Count: %u", 
             m, 
             g_model_system.models[m].name ? g_model_system.models[m].name : "NULL",
             g_model_system.models[m].isLoaded ? "TRUE" : "FALSE",
             g_model_system.models[m].lod_count);
    }

    // Inspect a few instances directly out of the spatial loop
    if (g_model_system.chunkCount > 0 && g_model_system.chunks[0].instanceCount > 0) {
        uint32_t test_global_idx = g_model_system.chunks[0].instanceOffset;
        CPUInstanceData* test_src = &g_model_system.instances[test_global_idx];
        LOGI("Sample Instance 0 -> model_index: %u", test_src->model_index);
    }*/

    // =========================================================================
    // --- PASS 2: Compute contiguous GPU offsets per Model + LOD combo ---
    // =========================================================================
    uint32_t current_buffer_offset = 0;
    for (uint32_t m = 0; m < g_model_system.modelCount; m++) {
        for (uint32_t l = 0; l < MAX_LOD_LEVELS; l++) {
            uint32_t slot = LOD_SLOT(m, l);
            // --- FIX 3: Access struct trackers ---
            g_model_system.model_lod_gpu_offsets[slot] = current_buffer_offset;
            current_buffer_offset += g_model_system.model_lod_visible_counts[slot];
            
            // Clear counter down to reuse as a write accumulator pointer in Pass 3
            g_model_system.model_lod_visible_counts[slot] = 0;
        }
    }

    // =========================================================================
    // --- PASS 3: Stream sorted instances into precise GPU slots ---
    // =========================================================================
    for (uint32_t i = 0; i < g_model_system.chunkCount; i++) {
        SpatialChunk* chunk = &g_model_system.chunks[i];

        float dx = chunk->center[0] - cam_x;
        float dy = chunk->center[1] - cam_y;
        float dz = chunk->center[2] - cam_z;
        float distance = sqrtf(dx * dx + dy * dy + dz * dz) - chunk->radius;

        bool pass_distance = !g_enable_distance_culling || (distance <= stream_radius);

        if (pass_distance) {
            for (uint32_t j = 0; j < chunk->instanceCount; j++) {
                uint32_t global_idx = chunk->instanceOffset + j;
                CPUInstanceData* cpu_src = &g_model_system.instances[global_idx];
                
                bool pass_frustum = !g_enable_frustum_culling || 
                                    is_visible_clip_space(cpu_src->bounding_center, cpu_src->bounding_radius);

                if (pass_frustum) {
                    // --- SAFETY CHECK 1: Match Pass 1 Array Boundaries ---
                    if (cpu_src->model_index >= g_model_system.modelCount) {
                        continue; 
                    }

                    Model* model = &g_model_system.models[cpu_src->model_index];

                    // --- TRAP CORRECTION: Skip processing safely if the engine hasn't fully registered this mesh footprint
                    if (!model->isLoaded) {
                        continue;
                    }

                    // Select LOD Level
                    uint32_t lod_level = 0;
                    if (distance > get_param_float(PARAM_RENDER_LOD3_DISTANCE))      lod_level = 3;
                    else if (distance > get_param_float(PARAM_RENDER_LOD2_DISTANCE)) lod_level = 2;
                    else if (distance > get_param_float(PARAM_RENDER_LOD1_DISTANCE)) lod_level = 1;
                    
                    // --- SAFETY CHECK 2: Match Underflow Guards ---
                    if (model->lod_count == 0) {
                        lod_level = 0; 
                    } else if (lod_level >= model->lod_count) {
                        lod_level = model->lod_count - 1;
                    }

                    uint32_t m_idx = cpu_src->model_index;
                    uint32_t slot = LOD_SLOT(m_idx, lod_level);
                    uint32_t max_allowed_slots = g_model_system.modelCount * MAX_LOD_LEVELS;
                    
                    // --- SAFETY CHECK 3: Array Range verification ---
                    if (slot >= max_allowed_slots) {
                        continue;
                    }
                    
                    // Calculate target location within this specific model's LOD bucket slice
                    uint32_t target_idx = g_model_system.model_lod_gpu_offsets[slot] + g_model_system.model_lod_visible_counts[slot];

                    InstanceData* gpu_dest = &g_model_system.visibleInstancesCPU[target_idx];
                    
                    memcpy(gpu_dest->model, cpu_src->model, sizeof(float) * 16);
                    memcpy(gpu_dest->color, cpu_src->color, sizeof(float) * 4);

                    g_model_system.model_lod_visible_counts[slot]++;
                    g_model_system.visibleCount++;
                }
            }
        }
    }

    g_culled_count = g_stats_distance_culled + g_stats_frustum_culled;

    // =========================================================================
    // --- PASS 4: Sync Device Buffers ---
    // =========================================================================
    if (g_model_system.visibleCount > 0 && g_model_system.mappedData != NULL) {
        memcpy(g_model_system.mappedData, 
               g_model_system.visibleInstancesCPU, 
               g_model_system.visibleCount * sizeof(InstanceData));

        VkMappedMemoryRange flushRange = {0};
        flushRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        flushRange.memory = g_model_system.instanceMemory;
        flushRange.offset = 0;
        flushRange.size = VK_WHOLE_SIZE;
        vkFlushMappedMemoryRanges(vk_device, 1, &flushRange);
    }
}