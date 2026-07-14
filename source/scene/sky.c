#include "pch.h"
#include "scene/sky.h"

#ifndef __EMSCRIPTEN__
    extern VkDevice vk_device;
    VkDeviceMemory skyUBOMemory = VK_NULL_HANDLE;
#endif

    SkyUBO skyUBOData = {0};

    void sky_update(void)
    {
        float* time_of_day = get_param_float_ptr(PARAM_SHADERS_SKY_TIME_OF_DAY);

        *time_of_day += 0.0005f * get_param_float(PARAM_SHADERS_SKY_DAY_NIGHT_CYCLE_SPEED);

        if (*time_of_day > 1.0f)
            *time_of_day -= 1.0f;

        static float accumTime = 0.0f;
        accumTime += 0.016f;

        skyUBOData.time = accumTime;
        skyUBOData.yaw = camera.yaw * 0.0174532925f;
        skyUBOData.pitch = camera.pitch * 0.0174532925f;

        skyUBOData.timeOfDay = get_param_float(PARAM_SHADERS_SKY_TIME_OF_DAY);
        skyUBOData.dayNightBlend = 1.0f; // to be removed

        skyUBOData.nebulaScale = get_param_float(PARAM_SHADERS_SKY_NEBULA_SCALE);
        skyUBOData.nebulaIntensity = get_param_float(PARAM_SHADERS_SKY_NEBULA_POWER);
        skyUBOData.nebulaLayerCount = get_param_float(PARAM_SHADERS_SKY_NEBULA_LAYERS);
        skyUBOData.nebulaSpeed = get_param_float(PARAM_SHADERS_SKY_NEBULA_SPEED);

        skyUBOData.starCount = get_param_float(PARAM_SHADERS_SKY_STAR_COUNT);
        skyUBOData.starBrightness = get_param_float(PARAM_SHADERS_SKY_STAR_POWER);
        skyUBOData.starTwinkleSpeed = get_param_float(PARAM_SHADERS_SKY_STAR_TWINKLE_SPEED);
        skyUBOData.starSize = get_param_float(PARAM_SHADERS_SKY_STAR_COUNT);

        skyUBOData.auroraIntensity = get_param_float(PARAM_SHADERS_SKY_AURORA_POWER);
        skyUBOData.auroraSpeed = get_param_float(PARAM_SHADERS_SKY_AURORA_SPEED);

        // Clear and Copy Colors (Ensure the 4th float is 1.0 or 0.0)
        memset(skyUBOData.nebulaColorNight, 0, sizeof(float)*4);
        memcpy(skyUBOData.nebulaColorNight, get_param_vec3_ptr(PARAM_SHADERS_SKY_NEBULA_COLOR_NIGHT), sizeof(float)*3);

        memset(skyUBOData.nebulaColorDay, 0, sizeof(float)*4);
        memcpy(skyUBOData.nebulaColorDay, get_param_vec3_ptr(PARAM_SHADERS_SKY_NEBULA_COLOR_DAY), sizeof(float)*3);

        memset(skyUBOData.auroraColor, 0, sizeof(float)*4);
        memcpy(skyUBOData.auroraColor, get_param_vec3_ptr(PARAM_SHADERS_SKY_AURORA_COLOR), sizeof(float)*3);

        skyUBOData.vignetteStrength = get_param_float(PARAM_SHADERS_SKY_VIGNETTE_POWER);
        skyUBOData.overallBrightness = get_param_float(PARAM_SHADERS_SKY_BRIGHTNESS);
        memcpy(skyUBOData.inverseView, cameraUBOData.inverseView, 16 * sizeof(float));
        skyUBOData.verticalBandFrequency = get_param_float(PARAM_SHADERS_SKY_VERTICAL_BAND_FREQUENCY);
    #ifndef __EMSCRIPTEN__
        void* data;
        vkMapMemory(vk_device, skyUBOMemory, 0, sizeof(SkyUBO), 0, &data);
        memcpy(data, &skyUBOData, sizeof(SkyUBO));
        vkUnmapMemory(vk_device, skyUBOMemory);
    #else
    // On WebGPU we do it in sky_webgpu.c: update_sky_ubo_webgpu() via wgpuQueueWriteBuffer
    #endif
    }