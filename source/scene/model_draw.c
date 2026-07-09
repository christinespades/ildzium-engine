#include "pch.h"
#include "scene/model_draw.h"
#ifdef __EMSCRIPTEN__
	extern WGPUDevice device;
	extern WGPURenderPipeline modelPipeline;
	extern WGPUBindGroupLayout    modelBindGroupLayout;

	WGPUBindGroup draw_models_webgpu(WGPURenderPassEncoder pass)
	{
	    if (g_model_system.modelCount == 0 || g_model_system.instanceCount == 0) return NULL;

		Model* m = &g_model_system.models[0];
	    
		static int debug_ticks = 0;
		if (m->mesh.vertexBuffer == NULL || m->mesh.vertexCount == 0) {
		    if (debug_ticks++ % 60 == 0) {
		        printf("[Draw Engine Warning]: Skipping mesh render. Status -> vertexBuffer: %p, vertexCount: %d\n", 
		               m->mesh.vertexBuffer, m->mesh.vertexCount);
		    }
		    return NULL;
		}

	    update_model_instances();

	    wgpuRenderPassEncoderSetPipeline(pass, modelPipeline);

	    // Create bind group (camera + lighting + instances)
	    WGPUBindGroupEntry entries[3] = {
	        { .binding = 0, .buffer = cameraBuffer,          .offset = 0, .size = sizeof(CameraUBO) },
	        { .binding = 1, .buffer = lightingBuffer,        .offset = 0, .size = sizeof(LightingUBO) },
	        { .binding = 2, .buffer = g_model_system.instanceBuffer,
	          .offset = 0, .size = (uint64_t)g_model_system.instanceCount * sizeof(InstanceData) }
	    };

	    WGPUBindGroupDescriptor bgDesc = {
	        .layout = modelBindGroupLayout,        // ← we will define this global
	        .entryCount = 3,
	        .entries = entries
	    };

	    WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(device, &bgDesc);

	    wgpuRenderPassEncoderSetBindGroup(pass, 0, bindGroup, 0, NULL);

	    // Draw the first model (you can extend this later to support multiple models)
	    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, m->mesh.vertexBuffer, 0, WGPU_WHOLE_SIZE);

	    if (m->mesh.indexCount > 0) {
	        wgpuRenderPassEncoderSetIndexBuffer(pass, m->mesh.indexBuffer,
	                                            WGPUIndexFormat_Uint32, 0, WGPU_WHOLE_SIZE);
	        wgpuRenderPassEncoderDrawIndexed(pass, m->mesh.indexCount,
	                                         g_model_system.instanceCount, 0, 0, 0);
	    } else {
	        wgpuRenderPassEncoderDraw(pass, m->mesh.vertexCount,
	                                  g_model_system.instanceCount, 0, 0);
	    }
	return bindGroup;
	}
#else
	void draw_models(VkCommandBuffer cmd)
	{
	    if (g_model_system.modelCount == 0 || g_model_system.instanceCount == 0) return;

	    update_model_instances();

	    // If everything got culled out of view, return early!
	    if (g_culled_count == g_model_system.instanceCount) return;

	    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);

	    VkDescriptorSet sets[2] = { g_lights.descriptorSet, modelDescriptorSet };
	    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
	                            modelPipelineLayout,
	                            0, 2, sets, 0, NULL);

	    // Loop through all active geometric types inside the model engine cache
	    for (uint32_t m_idx = 0; m_idx < g_model_system.modelCount; m_idx++) {
	        uint32_t active_instances = g_model_visible_counts[m_idx];
	        if (active_instances == 0) continue; // None visible on screen

	        Model* m = &g_model_system.models[m_idx];
	        if (m->mesh.vertexBuffer == VK_NULL_HANDLE) continue;

	        VkDeviceSize offset = 0;
	        vkCmdBindVertexBuffers(cmd, 0, 1, &m->mesh.vertexBuffer, &offset);

	        // Update the frame drawing counter globally
	        g_drawn_count += active_instances;

	        // Note the 4th parameter `firstInstance`: passing g_model_gpu_offsets[m_idx] 
	        // redirects gl_InstanceIndex in GLSL to fetch data correctly out of the packed SSBO storage buffer.
	        if (m->mesh.indexCount > 0) {
	            vkCmdBindIndexBuffer(cmd, m->mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	            vkCmdDrawIndexed(cmd, m->mesh.indexCount, active_instances, 0, 0, g_model_gpu_offsets[m_idx]);
	        } else {
	            vkCmdDraw(cmd, m->mesh.vertexCount, active_instances, 0, g_model_gpu_offsets[m_idx]);
	        }
	    }
	}
#endif