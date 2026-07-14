#include "pch.h"
#include "renderer_debug_vk_draw.h"

void renderer_debug_build(
    VkCommandBuffer cmd,
    uint32_t currentFrame)
{
    if(g_renderer_debug.primitiveCount == 0)
        return;

    renderer_debug_update_compute_descriptors(currentFrame);

    memcpy(
        g_renderer_debug.mappedPrimitiveBuffers[currentFrame],
        g_renderer_debug.primitiveCPU,
        sizeof(DebugPrimitiveCommand) *
        g_renderer_debug.primitiveCount);

    vkCmdBindPipeline(
        cmd,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        g_renderer_debug.computePipeline);

    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        g_renderer_debug.computePipelineLayout,
        0,
        1,
        &g_renderer_debug.computeDescriptorSets[currentFrame],
        0,
        NULL);

    vkCmdPushConstants(
        cmd,
        g_renderer_debug.computePipelineLayout,
        VK_SHADER_STAGE_COMPUTE_BIT,
        0,
        sizeof(uint32_t),
        &g_renderer_debug.primitiveCount);

    //LOGI("Primitive count = %u", g_renderer_debug.primitiveCount);
	//LOGI("Generated line count = %u", g_renderer_debug.generatedLineCount);

	vkCmdFillBuffer(
	    cmd,
	    g_renderer_debug.instanceBuffers[currentFrame],
	    0,
	    VK_WHOLE_SIZE,
	    0);

	VkBufferMemoryBarrier clearBarrier =
	{
	    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
	    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
	    .dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
	    .buffer = g_renderer_debug.instanceBuffers[currentFrame],
	    .offset = 0,
	    .size = VK_WHOLE_SIZE
	};

	vkCmdPipelineBarrier(
	    cmd,
	    VK_PIPELINE_STAGE_TRANSFER_BIT,
	    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	    0,
	    0, NULL,
	    1, &clearBarrier,
	    0, NULL);
	
    vkCmdDispatch(
        cmd,
        g_renderer_debug.primitiveCount,
        1,
        1);

    VkBufferMemoryBarrier barrier =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .srcAccessMask =
            VK_ACCESS_SHADER_WRITE_BIT,
        .dstAccessMask =
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
        .buffer =
            g_renderer_debug.instanceBuffers[currentFrame],
        .offset = 0,
        .size = VK_WHOLE_SIZE
    };

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        0,
        0,NULL,
        1,&barrier,
        0,NULL);
}

void renderer_debug_draw(
    VkCommandBuffer cmdBuffer,
    uint32_t currentFrame,
    VkExtent2D swapchainExtent)
{
	if (g_renderer_debug.primitiveCount == 0)
	    return;

    vec3 cameraPos = {
        camera.x,
        camera.y,
        camera.z
    };

    vkCmdBindPipeline(
        cmdBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        g_renderer_debug.pipeline
    );

    VkDescriptorSet sets[2] =
    {
        g_lights.descriptorSet,
        modelDescriptorSet
    };

    vkCmdBindDescriptorSets(
        cmdBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        modelPipelineLayout,
        0,
        2,
        sets,
        0,
        NULL
    );

    VkViewport viewport =
    {
        0,
        0,
        (float)swapchainExtent.width,
        (float)swapchainExtent.height,
        0,
        1
    };

    VkRect2D scissor =
    {
        {0,0},
        swapchainExtent
    };

    vkCmdSetViewport(
        cmdBuffer,
        0,
        1,
        &viewport
    );

    vkCmdSetScissor(
        cmdBuffer,
        0,
        1,
        &scissor
    );

    VkBuffer buffers[2] =
    {
        g_renderer_debug.quadVertexBuffer,
        g_renderer_debug.instanceBuffers[currentFrame]
    };

    VkDeviceSize offsets[2] =
    {
        0,
        0
    };

    vkCmdBindVertexBuffers(
        cmdBuffer,
        0,
        2,
        buffers,
        offsets
    );

    vkCmdDraw(
        cmdBuffer,
        4,
        g_renderer_debug.generatedLineCount,
        0,
        0
    );

	g_renderer_debug.primitiveCount = 0;
	g_renderer_debug.generatedLineCount = 0;
}