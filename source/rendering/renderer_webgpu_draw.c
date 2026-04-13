#include "pch.h"
#if defined(__EMSCRIPTEN__)
    #include "renderer_webgpu.h"
    #include "scene/camera.h"
    // in pch.h: #include <webgpu/webgpu.h>

    extern CameraUBO cameraUBOData;
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
        if (gpu_state != GPU_STATE_READY) {
            static int once = 0;
            if (once++ < 3) printf("GPU not ready\n");
            return;
        }

        static int frame = 0;
        frame++;
        if (frame % 240 == 0) printf("Drawing frame %d\n", frame);

        wgpuQueueWriteBuffer(queue, cameraBuffer, 0, &cameraUBOData, sizeof(CameraUBO));

        WGPUSurfaceTexture surfaceTexture = {0};
        wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);

        if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
            surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal)
        {
            printf("GetCurrentTexture failed - status: %d\n", surfaceTexture.status);
            if (surfaceTexture.texture) wgpuTextureRelease(surfaceTexture.texture);
            return;
        }

        WGPUTextureViewDescriptor viewDesc = {0};
        viewDesc.format          = wgpuTextureGetFormat(surfaceTexture.texture);
        viewDesc.dimension       = WGPUTextureViewDimension_2D;
        viewDesc.baseMipLevel    = 0;
        viewDesc.mipLevelCount   = 1;                         // critical
        viewDesc.baseArrayLayer  = 0;
        viewDesc.arrayLayerCount = 1;                         // critical
        viewDesc.aspect          = WGPUTextureAspect_All;
        viewDesc.label = (WGPUStringView){ .data = "BackbufferView", .length = WGPU_STRLEN };

        WGPUTextureView backbuffer = wgpuTextureCreateView(surfaceTexture.texture, &viewDesc);
        if (!backbuffer) {
            printf("Failed to create backbuffer view\n");
            wgpuTextureRelease(surfaceTexture.texture);
            return;
        }

        // printf("Rendering frame %d - clear color applied\n", frame); 

        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, NULL);

        WGPURenderPassColorAttachment colorAttachment = {0};
        colorAttachment.view = backbuffer;
        colorAttachment.loadOp = WGPULoadOp_Clear;
        colorAttachment.storeOp = WGPUStoreOp_Store;
        colorAttachment.clearValue = (WGPUColor){0.7f, 0.1f, 0.1f, 1.0f};

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

        // === IMPORTANT: NO wgpuSurfacePresent() on Web! ===

        // Cleanup
        wgpuTextureViewRelease(backbuffer);
        wgpuTextureRelease(surfaceTexture.texture);
        wgpuCommandBufferRelease(cmd);
        wgpuCommandEncoderRelease(encoder);
    }
#endif