#if defined(__EMSCRIPTEN__)
#include "pch.h"
#include "renderer_webgpu.h"
#include "scene/camera.h"
// in pch.h: #include <webgpu/webgpu.h>

extern CameraUBO cameraUBOData;     // already declared in renderer_webgpu.h probably
extern WGPUBuffer cameraBuffer;

// These globals are defined in renderer_webgpu.c
extern WGPUDevice        device;
extern WGPUQueue         queue;
extern WGPUSurface       surface;
extern WGPURenderPipeline pipeline;
extern WGPUBuffer        vertexBuffer;
extern WGPUBindGroup     bindGroup;


void webgpu_draw(void)
{
    if (!device || !surface) {
        return;
    }

    wgpuQueueWriteBuffer(queue, cameraBuffer, 0, &cameraUBOData, sizeof(CameraUBO));

    WGPUSurfaceTexture surfaceTexture = {0};
    wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);

    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
        surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
        if (surfaceTexture.texture) wgpuTextureRelease(surfaceTexture.texture);
        return;
    }

    WGPUTextureViewDescriptor viewDesc = {0};
    WGPUTextureView backbuffer = wgpuTextureCreateView(surfaceTexture.texture, &viewDesc);
    if (!backbuffer) {
        wgpuTextureRelease(surfaceTexture.texture);
        return;
    }

    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, NULL);

    WGPURenderPassColorAttachment colorAttachment = {0};
    colorAttachment.view = backbuffer;
    colorAttachment.loadOp = WGPULoadOp_Clear;
    colorAttachment.storeOp = WGPUStoreOp_Store;
    colorAttachment.clearValue = (WGPUColor){0.1f, 0.1f, 0.2f, 1.0f};

    WGPURenderPassDescriptor passDesc = {0};
    passDesc.colorAttachmentCount = 1;
    passDesc.colorAttachments = &colorAttachment;

    WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &passDesc);

    wgpuRenderPassEncoderSetPipeline(pass, pipeline);
    wgpuRenderPassEncoderSetBindGroup(pass, 0, bindGroup, 0, NULL);
    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, vertexBuffer, 0, sizeof(float)*9);
    wgpuRenderPassEncoderDraw(pass, 3, 1, 0, 0);

    wgpuRenderPassEncoderEnd(pass);

    WGPUCommandBuffer cmd = wgpuCommandEncoderFinish(encoder, NULL);
    wgpuQueueSubmit(queue, 1, &cmd);

    wgpuSurfacePresent(surface);

    wgpuTextureViewRelease(backbuffer);
    wgpuTextureRelease(surfaceTexture.texture);
    wgpuCommandBufferRelease(cmd);
    wgpuCommandEncoderRelease(encoder);
}
#endif