#include "pch.h"
#include "scene/model_instances.h"
#ifdef __EMSCRIPTEN__
	extern WGPUDevice device;
#endif

#ifndef __EMSCRIPTEN__
	void create_instance_buffer(uint32_t capacity)
	{
	    VkDeviceSize newSize = (VkDeviceSize)capacity * sizeof(InstanceData);

	    // Destroy old buffer if it exists
	    if (g_model_system.instanceBuffer != VK_NULL_HANDLE) {
	        vkDestroyBuffer(vk_device, g_model_system.instanceBuffer, NULL);
	        vkFreeMemory(vk_device, g_model_system.instanceMemory, NULL);
	    }

	    create_vulkan_buffer(newSize,
	                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
	                         &g_model_system.instanceBuffer,
	                         &g_model_system.instanceMemory);

	    g_model_system.instanceBufferSize = newSize;
	    g_model_system.instanceCapacity = capacity;
	}
#endif

void add_model_instance(const char* model_name, const float transform[16], const float color[4])
{
    // Load (or find) the model ONLY ONCE
    static Model* cached_model = NULL;
    static const char* last_name = NULL;

    if (cached_model == NULL || strcmp(model_name, last_name) != 0) {
        cached_model = find_or_load_model(model_name);
        last_name = model_name;               // safe because path is constant string
    }

    if (!cached_model) return;

    // Grow instance array if needed
    if (g_model_system.instanceCount >= g_model_system.instanceCapacity) {
        g_model_system.instanceCapacity *= 2;
        if (g_model_system.instanceCapacity == 0) g_model_system.instanceCapacity = 256;
        g_model_system.instances = realloc(g_model_system.instances,
                                           g_model_system.instanceCapacity * sizeof(InstanceData));
    }

    uint32_t idx = g_model_system.instanceCount++;
    InstanceData* inst = &g_model_system.instances[idx];

    memcpy(inst->model, transform, 16 * sizeof(float));
    memcpy(inst->color, color, 4 * sizeof(float));

    cached_model->instanceCount++;   // bookkeeping
}

void update_model_instances(void)
{
    if (g_model_system.instanceCount == 0) return;

#ifdef __EMSCRIPTEN__
    uint64_t needed = (uint64_t)g_model_system.instanceCount * sizeof(InstanceData);
    if (needed > wgpuBufferGetSize(g_model_system.instanceBuffer)) {
        // Recreate larger buffer (for simplicity - you can optimize later)
        wgpuBufferRelease(g_model_system.instanceBuffer);
        WGPUBufferDescriptor desc = {
            .usage = WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst,
            .size = needed * 2
        };
        g_model_system.instanceBuffer = wgpuDeviceCreateBuffer(device, &desc);
    }

    // Upload (no transpose needed in WGSL if you use row-major or adjust shader)
    wgpuQueueWriteBuffer(queue, g_model_system.instanceBuffer, 0,
                         g_model_system.instances, 
                         g_model_system.instanceCount * sizeof(InstanceData));
#else
    VkDeviceSize needed = (VkDeviceSize)g_model_system.instanceCount * sizeof(InstanceData);
    if (needed > g_model_system.instanceBufferSize) {
        create_instance_buffer(g_model_system.instanceCount * 2);
        update_model_descriptor();
    }

    void* data;
    vkMapMemory(vk_device, g_model_system.instanceMemory, 0, needed, 0, &data);
    
    // Transpose each model matrix while copying (optimal place)
    InstanceData* gpuData = (InstanceData*)data;
    for (uint32_t i = 0; i < g_model_system.instanceCount; ++i) {
        const InstanceData* cpuInst = &g_model_system.instances[i];

        // Transpose model matrix (row-major → column-major for GLSL)
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                gpuData[i].model[col * 4 + row] = cpuInst->model[row * 4 + col];
            }
        }

        // Copy color as-is
        gpuData[i].color[0] = cpuInst->color[0];
        gpuData[i].color[1] = cpuInst->color[1];
        gpuData[i].color[2] = cpuInst->color[2];
        gpuData[i].color[3] = cpuInst->color[3];
    }
    vkUnmapMemory(vk_device, g_model_system.instanceMemory);
#endif
}
