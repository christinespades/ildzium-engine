#include "pch.h"
#if defined(__EMSCRIPTEN__)
    #include "rendering/renderer_webgpu_draw.h"
    extern CameraUBO cameraUBOData;
    extern WGPUBuffer cameraBuffer;
    extern WGPUTextureFormat swapchainFormat;
    extern void sky_update();

    extern WGPUDevice        device;
    extern WGPUInstance instance;
    extern WGPUQueue         queue;
    extern WGPUSurface       surface;
    extern WGPUTextureView depthView;
    extern WGPUTexture depthTexture;
    extern int currentDepthWidth;
    extern int currentDepthHeight;

    static void onPopErrorScope(WGPUPopErrorScopeStatus status, WGPUErrorType type, WGPUStringView message, void* userdata1, void* userdata2)
    {
        if (type != WGPUErrorType_NoError && message.data) {
            printf("🚨 WebGPU Error: %.*s\n", (int)message.length, message.data);
        }
    }

    void ensure_depth_texture(void)
    {
        if (depthTexture && currentDepthWidth == g_width && currentDepthHeight == g_height) {
            return;
        }

        // Cleanup old
        if (depthView)  wgpuTextureViewRelease(depthView);
        if (depthTexture) wgpuTextureRelease(depthTexture);

        WGPUTextureDescriptor depthDesc = {0};
        depthDesc.usage = WGPUTextureUsage_RenderAttachment;
        depthDesc.dimension = WGPUTextureDimension_2D;
        depthDesc.size.width = g_width;
        depthDesc.size.height = g_height;
        depthDesc.size.depthOrArrayLayers = 1;
        depthDesc.format = WGPUTextureFormat_Depth24Plus;   // or Depth32Float
        depthDesc.mipLevelCount = 1;
        depthDesc.sampleCount = 1;

        depthTexture = wgpuDeviceCreateTexture(device, &depthDesc);
        depthView = wgpuTextureCreateView(depthTexture, NULL);

        currentDepthWidth = g_width;
        currentDepthHeight = g_height;

        printf("Depth texture recreated: %d x %d\n", g_width, g_height);
    }
    
    void webgpu_draw(float dt)
    {
        if (gpu_state != GPU_STATE_READY) return;

        /* printf("FRAME DEBUG: Models: %d, Instances: %d, Captured: %d\n", 
               g_model_system.modelCount, 
               g_model_system.instanceCount, 
               g_ui_ctx->cursor_captured); */

        static int frame = 0;
        frame++;
        if (frame % 60 == 0) printf("Drawing frame %d | %dx%d\n", frame, g_width, g_height);

        wgpuInstanceProcessEvents(instance);

        static int lastWidth = 0, lastHeight = 0;
        if (g_width != lastWidth || g_height != lastHeight || lastWidth == 0) {
            WGPUSurfaceConfiguration config = {0};
            config.device = device;
            config.format = swapchainFormat;
            config.usage = WGPUTextureUsage_RenderAttachment;
            config.width = g_width;
            config.height = g_height;
            config.presentMode = WGPUPresentMode_Fifo;
            wgpuSurfaceConfigure(surface, &config);
            lastWidth = g_width;
            lastHeight = g_height;
            printf("Surface reconfigured to %dx%d\n", g_width, g_height);
        }

        ensure_depth_texture();

        WGPUSurfaceTexture surfaceTexture = {0};
        wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);

        if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
            surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
            printf("GetCurrentTexture failed: %d\n", surfaceTexture.status);
            if (surfaceTexture.texture) wgpuTextureRelease(surfaceTexture.texture);
            return;
        }

        WGPUTextureView backbuffer = wgpuTextureCreateView(surfaceTexture.texture, NULL);
        if (!backbuffer) {
            wgpuTextureRelease(surfaceTexture.texture);
            return;
        }

        wgpuDevicePushErrorScope(device, WGPUErrorFilter_Validation);

        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, NULL);

        float t = frame * 0.01f;
        float r = 0.5f + 0.5f * sin(t * 1.7f);
        float g = 0.5f + 0.5f * sin(t * 2.3f + 2.0f);
        float b = 0.5f + 0.5f * sin(t * 3.1f + 4.0f);
        float pulse = powf(0.5f + 0.5f * sin(t * 0.8f), 3.0f);
        float darkness = 0.45f + 0.85f * (0.5f + 0.5f * sin(t * 0.25f));

        WGPURenderPassColorAttachment colorAttachment = {0};
        colorAttachment.view = backbuffer;
        colorAttachment.loadOp = WGPULoadOp_Clear;
        colorAttachment.storeOp = WGPUStoreOp_Store;
        colorAttachment.clearValue = (WGPUColor){
            r * pulse * darkness,
            g * pulse * darkness,
            b * pulse * darkness,
            1.0
        };
        colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

        WGPURenderPassDepthStencilAttachment depthAttachment = {0};
        depthAttachment.view = depthView;
        depthAttachment.depthLoadOp = WGPULoadOp_Clear;
        depthAttachment.depthStoreOp = WGPUStoreOp_Store;
        depthAttachment.depthClearValue = 1.0f;

        WGPURenderPassDescriptor passDesc = {0};
        passDesc.colorAttachmentCount = 1;
        passDesc.colorAttachments = &colorAttachment;
        passDesc.depthStencilAttachment = &depthAttachment;

        WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &passDesc);

        wgpuRenderPassEncoderSetViewport(pass, 0, 0, g_width, g_height, 0.0f, 1.0f);
        /*if (frame % 60 == 0) {
            printf("[CANVAS METRICS] Viewport width: %d, Viewport height: %d\n", g_width, g_height);
        }*/
        wgpuRenderPassEncoderSetScissorRect(pass, 0, 0, g_width, g_height);

        update_sky_ubo_webgpu();
        sky_draw_webgpu(pass);
        update_camera_ubo();
        lights_update(
              0.15f, 0.15f, 0.15f,
              1.0f, 1.0f, -1.0f,
              1.0f,
              4,
              camera.x, camera.y, camera.z);
        // Track the bind group returned by the model draw pass
        WGPUBindGroup modelBindGroup = draw_models_webgpu(pass);

        if (g_ui_ctx->cursor_captured) {
            ui_draw(g_ui_ctx, ui_framebuffer, g_width, g_height, dt);
            ui_renderer_upload(ui_framebuffer, g_width, g_height);
            ui_renderer_draw(pass);
        }
        //printf("width: %d\n", g_width);

        wgpuRenderPassEncoderEnd(pass);

        WGPUCommandBuffer cmd = wgpuCommandEncoderFinish(encoder, NULL);
        wgpuQueueSubmit(queue, 1, &cmd);

        WGPUPopErrorScopeCallbackInfo callbackInfo = {0};
        callbackInfo.callback = onPopErrorScope;
        callbackInfo.mode = WGPUCallbackMode_AllowProcessEvents; 
        callbackInfo.userdata1 = NULL;
        callbackInfo.userdata2 = NULL;

        wgpuDevicePopErrorScope(device, callbackInfo);

        if (modelBindGroup != NULL) {
            wgpuBindGroupRelease(modelBindGroup);
        }
        wgpuTextureViewRelease(backbuffer);
        wgpuCommandBufferRelease(cmd);
        wgpuCommandEncoderRelease(encoder);
        wgpuTextureRelease(surfaceTexture.texture);
    }
#endif