#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "scene/camera.h"
#include "rendering/device.h"
#include "scene/model.h"
#include "rendering/renderer_draw.h"
#include "scene/sky.h"
#include "ui/ui.h"
#include "ui/ui_renderer.h"

float g_fps = 0.0f;
VkSemaphore imageAvailable[MAX_FRAMES_IN_FLIGHT];
VkSemaphore renderFinished[MAX_FRAMES_IN_FLIGHT];
VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
uint32_t currentFrame = 0;
VkRenderPass renderPass = VK_NULL_HANDLE;
extern VkPipeline modelPipeline;
extern VkSwapchainKHR swapchain;
extern VkExtent2D swapchainExtent;
extern VkFramebuffer* framebuffers;
extern void sky_draw();
extern void sky_update();
extern void update_camera_ubo(void);

void draw_frame()
{
    double now = glfwGetTime();
    static double last_time = 0.0;

    double dt = now - last_time;
    last_time = now;

    // Moving average FPS (true render FPS)
    if (dt > 0.0)
    {
        float inst_fps = (float)(1.0 / dt);
        g_fps = (g_fps == 0.0f) ? inst_fps
                               : (0.9f * g_fps + 0.1f * inst_fps);
    }

    // Wait for the current frame to be free
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    // Acquire next image from swapchain
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                                            imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        return; // recreate swapchain here if needed

    // Reset and record command buffer for this frame
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo);

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

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

    // 1. Sky (now handled in its own module)
    sky_update();
    sky_draw(commandBuffers[currentFrame]);

    update_camera_ubo();

    // 2. 3D Models
    if (g_model_system.modelCount > 0) {
        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);

        draw_models(commandBuffers[currentFrame]);
    }

    if (g_ui_ctx->cursor_captured) {
        ui_draw(g_ui_ctx, ui_framebuffer, swapchainExtent.width, swapchainExtent.height);
        ui_renderer_upload(ui_framebuffer, swapchainExtent.width, swapchainExtent.height);
        ui_renderer_draw(commandBuffers[currentFrame]);
    }

    vkCmdEndRenderPass(commandBuffers[currentFrame]);
    vkEndCommandBuffer(commandBuffers[currentFrame]);

    // Submit command buffer
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submit = {0};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &imageAvailable[currentFrame];
    submit.pWaitDstStageMask = waitStages;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &commandBuffers[currentFrame];
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &renderFinished[currentFrame];

    vkQueueSubmit(graphicsQueue, 1, &submit, inFlightFences[currentFrame]);

    // Present frame
    VkPresentInfoKHR present = {0};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &renderFinished[currentFrame];
    present.swapchainCount = 1;
    present.pSwapchains = &swapchain;
    present.pImageIndices = &imageIndex;

    vkQueuePresentKHR(graphicsQueue, &present);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}