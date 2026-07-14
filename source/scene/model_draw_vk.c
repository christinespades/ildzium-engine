#include "pch.h"
#ifndef __EMSCRIPTEN__
	#include "scene/model_draw_vk.h"
	void draw_models(VkCommandBuffer cmd)
	{
	    if (g_model_system.modelCount == 0 || g_model_system.instanceCount == 0) return;
	    if (g_culled_count == g_model_system.instanceCount) return;

	    // 1. Bind Graphics Pipeline context
	    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);

	    // 2. Bind Layout Descriptors
	    VkDescriptorSet sets[2] = { g_lights.descriptorSet, modelDescriptorSet };
	    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
	                            modelPipelineLayout, 0, 2, sets, 0, NULL);

	    // 3. MASTER BUFFER BIND: Bind the entire scene's geometry pipelines once
	    VkDeviceSize vertex_buffer_offset = 0;
	    vkCmdBindVertexBuffers(cmd, 0, 1, &g_model_system.masterVertexBuffer, &vertex_buffer_offset);
	    vkCmdBindIndexBuffer(cmd, g_model_system.masterIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

	    uint32_t total_dispatched_instances = 0;

	    for (uint32_t m = 0; m < g_model_system.modelCount; m++) {
	        Model* model = &g_model_system.models[m];
	        if (!model->isLoaded) continue;

	        for (uint32_t l = 0; l < model->lod_count; l++) {
	            uint32_t slot = LOD_SLOT(m, l);
	            
	            // --- FIX: Extract trackers cleanly out of the tracking struct context ---
	            uint32_t visible_count = g_model_system.model_lod_visible_counts[slot];

	            if (visible_count == 0) continue;

	            MeshGeometry* mesh = &model->mesh_levels[l];
	            uint32_t gpu_first_instance_offset = g_model_system.model_lod_gpu_offsets[slot];

	            total_dispatched_instances += visible_count;

	            // 4. Draw directly using relative offsets inside the bound master blocks!
	            vkCmdDrawIndexed(
	                cmd,
	                mesh->indexCount,            // Total indices inside this discrete LOD mesh asset
	                visible_count,               // Active instance count within distance profile
	                mesh->firstIndex,            // Core position inside global index sequence array
	                mesh->vertexOffset,          // Structural base address shift in global vertex arena
	                gpu_first_instance_offset    // BaseInstance reading index offset mapping to GPU storage
	            );
	        }
	    }

	    g_drawn_count = total_dispatched_instances;
	}
#endif