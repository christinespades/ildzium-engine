#include "pch.h"
#ifndef __EMSCRIPTEN__
    #include "rendering/renderer_vk_draw.h"

    extern float g_fps;
    VkSemaphore imageAvailable[RENDER_MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinished[RENDER_MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[RENDER_MAX_FRAMES_IN_FLIGHT];
    VkCommandBuffer commandBuffers[RENDER_MAX_FRAMES_IN_FLIGHT];
    VkRenderPass renderPass = VK_NULL_HANDLE;
    extern VkPipeline modelPipeline;
    extern VkSwapchainKHR swapchain;
    extern VkExtent2D swapchainExtent;
    extern VkFramebuffer* framebuffers;
    extern void sky_draw();
    extern void sky_update();
    extern void update_camera_ubo(void);

    void vulkan_draw()
    {
        double now = glfwGetTime();
        static double last_time = 0.0;

        g_dt = now - last_time;
        last_time = now;

        // Moving average FPS (true render FPS)
        if (g_dt > 0.0)
        {
            float inst_fps = (float)(1.0 / g_dt);
            g_fps = (g_fps == 0.0f) ? inst_fps
                                   : (0.9f * g_fps + 0.1f * inst_fps);
        }

        vkWaitForFences(vk_device, 1, &inFlightFences[g_current_frame], VK_TRUE, UINT64_MAX);
        vkResetFences(vk_device, 1, &inFlightFences[g_current_frame]);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(vk_device, swapchain, UINT64_MAX,
                                                imageAvailable[g_current_frame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            recreate_swapchain();
            return;
        }

        // Reset and record command buffer for this frame
        vkResetCommandBuffer(commandBuffers[g_current_frame], 0);

        VkCommandBufferBeginInfo beginInfo = {0};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(commandBuffers[g_current_frame], &beginInfo);

        VkRenderPassBeginInfo rpBegin = {0};
        rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpBegin.renderPass = renderPass;
        rpBegin.framebuffer = framebuffers[imageIndex];
        rpBegin.renderArea.extent = swapchainExtent;
        VkClearValue clearValues[2] = {
            {{{0.0f, 0.0f, 0.0f, 1.0f}}},   // color
            {{{1.0f, 0}}}                    // depth = 1.0, stencil = 0
        };
        rpBegin.clearValueCount = 2;
        rpBegin.pClearValues = clearValues;


        if (get_param_bool(PARAM_RENDER_DEBUG_ON)) {
            renderer_debug_add_primitives();
            //LOGI("primitiveCount = %u", g_renderer_debug.primitiveCount);
            renderer_debug_build(commandBuffers[g_current_frame], g_current_frame);
        }
            //debug_add_grid(get_param_float(PARAM_RENDER_DEBUG_GRID_LINE_WIDTH));
        vkCmdBeginRenderPass(commandBuffers[g_current_frame], &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

        sky_update();
        sky_draw(commandBuffers[g_current_frame]);

        update_camera_ubo();
        lights_update(commandBuffers[g_current_frame],
                      modelPipelineLayout, //binding 0 is lights UBO, 1 is camera + instance UBO
                      0.15f, 0.15f, 0.15f,
                      1.0f, 1.0f, -1.0f,
                      1.0f,
                      4,
                      camera.x, camera.y, camera.z);

        // 2. 3D Models
        if (g_model_system.modelCount > 0) {
            draw_models(commandBuffers[g_current_frame]);
        }

        if (get_param_bool(PARAM_RENDER_DEBUG_ON)) {
            renderer_debug_draw(commandBuffers[g_current_frame], g_current_frame, swapchainExtent);
        }

        if (g_ui_ctx->cursor_captured) {
            ui_draw();
            ui_renderer_upload();
            ui_renderer_draw(commandBuffers[g_current_frame]);
        }

        vkCmdEndRenderPass(commandBuffers[g_current_frame]);
        vkEndCommandBuffer(commandBuffers[g_current_frame]);

        // Submit command buffer
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo submit = {0};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &imageAvailable[g_current_frame];
        submit.pWaitDstStageMask = waitStages;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &commandBuffers[g_current_frame];
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &renderFinished[g_current_frame];

        vkQueueSubmit(graphicsQueue, 1, &submit, inFlightFences[g_current_frame]);


        vkQueueWaitIdle(graphicsQueue);

        /*DebugLineInstance *lines =
            (DebugLineInstance*)g_renderer_debug.mappedInstanceBuffers[g_current_frame];

        LOGI("%f %f %f",
            lines[0].start.x,
            lines[0].start.y,
            lines[0].start.z);
        LOGI("%f %f %f",
            lines[0].color.x,
            lines[0].color.y,
            lines[0].color.z);*/


        VkPresentInfoKHR present = {0};
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &renderFinished[g_current_frame];
        present.swapchainCount = 1;
        present.pSwapchains = &swapchain;
        present.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(graphicsQueue, &present);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || 
            result == VK_SUBOPTIMAL_KHR || 
            framebufferResized) 
        {
            framebufferResized = 0;
            recreate_swapchain();
        }
        else if (result != VK_SUCCESS)
        {
            LOGE("FATAL ERROR IN VULKAN IMAGE PRESENT");
        }

        g_current_frame = (g_current_frame + 1) % RENDER_MAX_FRAMES_IN_FLIGHT;
    }
#endif