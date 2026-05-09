#include "pch.h"
#include "scene/model_draw.h"
#ifdef __EMSCRIPTEN__
	extern WGPUDevice device;
	extern WGPURenderPipeline modelPipeline;
	extern WGPUBindGroupLayout    modelBindGroupLayout;

	void draw_models_webgpu(WGPURenderPassEncoder pass)
	{
	    if (g_model_system.modelCount == 0 || g_model_system.instanceCount == 0) return;

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
	    Model* m = &g_model_system.models[0];

	    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, m->mesh.vertexBuffer, 0, WGPU_WHOLE_SIZE);
	    wgpuRenderPassEncoderSetVertexBuffer(pass, 1, g_model_system.instanceBuffer, 0, WGPU_WHOLE_SIZE);

	    if (m->mesh.indexCount > 0) {
	        wgpuRenderPassEncoderSetIndexBuffer(pass, m->mesh.indexBuffer,
	                                            WGPUIndexFormat_Uint32, 0, WGPU_WHOLE_SIZE);
	        wgpuRenderPassEncoderDrawIndexed(pass, m->mesh.indexCount,
	                                         g_model_system.instanceCount, 0, 0, 0);
	    } else {
	        wgpuRenderPassEncoderDraw(pass, m->mesh.vertexCount,
	                                  g_model_system.instanceCount, 0, 0);
	    }

	    wgpuBindGroupRelease(bindGroup);   // safe to release immediately
	}
#else
    void draw_models(VkCommandBuffer cmd)
    {
        if (g_model_system.modelCount == 0 || g_model_system.instanceCount == 0) return;

        update_model_instances();

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);

        VkDescriptorSet sets[2] = { g_lights.descriptorSet, modelDescriptorSet };

        // --- Bind descriptor (set 0, binding 0) ---
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                modelPipelineLayout,
                                0, 2, sets, 0, NULL);

        // For now we only support ONE model (the first one). We'll improve this soon.
        Model* m = &g_model_system.models[0];
        if (m->mesh.vertexBuffer == VK_NULL_HANDLE) return;

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &m->mesh.vertexBuffer, &offset);

        if (m->mesh.indexCount > 0) {
            vkCmdBindIndexBuffer(cmd, m->mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, m->mesh.indexCount, g_model_system.instanceCount, 0, 0, 0);
        } else {
            vkCmdDraw(cmd, m->mesh.vertexCount, g_model_system.instanceCount, 0, 0);
        }
    }
#endif