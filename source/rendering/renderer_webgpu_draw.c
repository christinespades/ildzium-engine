#include "pch.h"
#if defined(__EMSCRIPTEN__)
    #include "rendering/renderer_webgpu_draw.h"
    extern CameraUBO cameraUBOData;
    extern WGPUBuffer cameraBuffer;
    extern WGPUTextureFormat swapchainFormat;
    extern void sky_update();

    // These globals are defined in renderer_webgpu.c
    extern WGPUDevice        device;
    extern WGPUQueue         queue;
    extern WGPUSurface       surface;
    extern WGPURenderPipeline pipeline;
    extern WGPUBuffer        vertexBuffer;
    extern WGPUBindGroup     bindGroup;

    static void onPopErrorScope(WGPUPopErrorScopeStatus status, WGPUErrorType type, WGPUStringView message, void* userdata1, void* userdata2)
    {
        if (type != WGPUErrorType_NoError && message.data) {
            printf("🚨 WebGPU Error: %.*s\n", (int)message.length, message.data);
        }
    }

    void webgpu_draw(float dt)
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

        // You can replace the manual viewDesc setup with this:
        WGPUTextureView backbuffer = wgpuTextureCreateView(surfaceTexture.texture, NULL);
        if (!backbuffer) {
            printf("Failed to create backbuffer view\n");
            wgpuTextureRelease(surfaceTexture.texture);
            return;
        }

        // printf("Rendering frame %d - clear color applied\n", frame); 

        WGPUCommandEncoderDescriptor encDesc = {
            .label = (WGPUStringView){ .data = "MainFrameEncoder", .length = 16 }
        };
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encDesc);

        WGPURenderPassColorAttachment colorAttachment = {0};
        colorAttachment.view = backbuffer;
        colorAttachment.loadOp = WGPULoadOp_Clear;
        colorAttachment.storeOp = WGPUStoreOp_Store;
        colorAttachment.clearValue = (WGPUColor){ 
            (double)sin(frame * 0.01), 
            0.1, 
            0.1, 
            1.0 
        };
        colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

        WGPURenderPassDescriptor passDesc = {0};
        passDesc.label = (WGPUStringView){ .data = "MainRenderPass", .length = 14 };
        passDesc.colorAttachmentCount = 1;
        passDesc.colorAttachments = &colorAttachment;

        update_sky_ubo_webgpu();

        sky_update();
        update_camera_ubo();
        lights_update(0.15f, 0.15f, 0.15f,      // ambient
                      1.0f, 1.0f, -1.0f,        // dir direction
                      1.0f,                     // dir intensity
                      4,                        // num point lights
                      camera.x, camera.y, camera.z);   // view pos

        WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &passDesc);
        wgpuRenderPassEncoderSetViewport(pass, 0, 0, g_width, g_height, 0.0f, 1.0f);
        wgpuRenderPassEncoderSetScissorRect(pass, 0, 0, g_width, g_height);

        wgpuRenderPassEncoderSetPipeline(pass, pipeline);
        wgpuRenderPassEncoderSetBindGroup(pass, 0, bindGroup, 0, NULL);
        wgpuRenderPassEncoderSetVertexBuffer(pass, 0, vertexBuffer, 0, WGPU_WHOLE_SIZE);
        wgpuRenderPassEncoderDraw(pass, 3, 1, 0, 0);

        sky_draw_webgpu(pass);
        draw_models_webgpu(pass);

        if (g_ui_ctx->cursor_captured) {
            ui_draw(g_ui_ctx, ui_framebuffer, g_width, g_height, dt);
            ui_renderer_upload(ui_framebuffer, g_width, g_height);
            ui_renderer_draw(pass);
        }
        //printf("width: %d\n", g_width);

        wgpuRenderPassEncoderEnd(pass);

        WGPUCommandBuffer cmd = wgpuCommandEncoderFinish(encoder, NULL);
        wgpuDevicePushErrorScope(device, WGPUErrorFilter_Validation);
        wgpuQueueSubmit(queue, 1, &cmd);
        WGPUPopErrorScopeCallbackInfo popInfo = {0};
        popInfo.mode = WGPUCallbackMode_AllowProcessEvents;
        popInfo.callback = onPopErrorScope;

        wgpuDevicePopErrorScope(device, popInfo);
        wgpuTextureViewRelease(backbuffer);
        wgpuTextureRelease(surfaceTexture.texture);
        wgpuCommandBufferRelease(cmd);
        wgpuCommandEncoderRelease(encoder);
    }
#endif