#include "pch.h"

#ifdef __EMSCRIPTEN__
    #include "scene/lights_webgpu.h"
    #include "rendering/renderer_webgpu_draw.h"   // for device + queue

    WGPUBuffer lightingBuffer = NULL;
    static LightingUBO lightingData = {0};   // CPU-side data

    void init_lights_webgpu(WGPUDevice device)
    {
        WGPUBufferDescriptor desc = {
            .label = (WGPUStringView){.data = "LightingUBO", .length = 11},
            .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
            .size = sizeof(LightingUBO),
            .mappedAtCreation = false
        };

        lightingBuffer = wgpuDeviceCreateBuffer(device, &desc);
    }

    void lights_update(float ambientR, float ambientG, float ambientB,
                       float dirX, float dirY, float dirZ,
                       float dirIntensity,
                       int numPoints,
                       float camX, float camY, float camZ)
    {
        LightingUBO* ubo = &lightingData;

        // Ambient
        ubo->ambient = (Vec3Std140){ ambientR, ambientG, ambientB, 0.0f };

        // Directional light
        ubo->dirLight.direction = (Vec3Std140){ dirX, dirY, dirZ, 0.0f };
        ubo->dirLight.color     = (Vec3Std140){ 1.0f, 1.0f, 1.0f, 0.0f };
        ubo->dirLight.intensity = dirIntensity;

        ubo->numPointLights = numPoints;

        // Point lights - same as your Vulkan version
        ubo->pointLights[0] = (PointLightStd140){
            .position = {5.0f, 3.0f, -2.0f, 0.0f},
            .color = {1.0f, 0.8f, 0.6f, 0.0f},
            .intensity = 1.5f, .constant = 1.0f,
            .linear = 0.09f, .quadratic = 0.032f
        };
        ubo->pointLights[1] = (PointLightStd140){
            .position = {-4.0f, 2.0f, 1.0f, 0.0f},
            .color = {0.6f, 0.8f, 1.0f, 0.0f},
            .intensity = 1.2f, .constant = 1.0f,
            .linear = 0.14f, .quadratic = 0.07f
        };
        ubo->pointLights[2] = (PointLightStd140){
            .position = {0.0f, 4.0f, 0.0f, 0.0f},
            .color = {0.6f, 1.0f, 0.6f, 0.0f},
            .intensity = 2.0f, .constant = 1.0f,
            .linear = 0.07f, .quadratic = 0.017f
        };
        ubo->pointLights[3] = (PointLightStd140){
            .position = {2.0f, 1.0f, 5.0f, 0.0f},
            .color = {1.0f, 0.4f, 0.4f, 0.0f},
            .intensity = 1.0f, .constant = 1.0f,
            .linear = 0.09f, .quadratic = 0.032f
        };

        // Disable unused point lights
        for (int i = numPoints; i < 4; ++i) {
            memset(&ubo->pointLights[i], 0, sizeof(PointLightStd140));
        }

        // Camera position
        ubo->viewPos = (Vec3Std140){ camX, camY, camZ, 0.0f };

        // Upload to GPU
        wgpuQueueWriteBuffer(queue, lightingBuffer, 0, ubo, sizeof(LightingUBO));
    }
#endif