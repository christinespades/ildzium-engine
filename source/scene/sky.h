#pragma once
#include "scene/camera.h"
#include "ui/ui.h"
#include "core/params/params.h"

#ifndef __EMSCRIPTEN__
    extern VkDevice vk_device;
    extern VkDeviceMemory skyUBOMemory;
#endif

typedef struct {
    float time;
    float yaw;
    float pitch;
    float timeOfDay;
    float dayNightBlend;
    float nebulaSpeed;
    float nebulaScale;
    float nebulaIntensity;
    float nebulaLayerCount;
    float starCount;
    float starBrightness;
    float starTwinkleSpeed;
    float starSize;
    float auroraIntensity;
    float auroraSpeed;
    float pad0;               // 16-byte alignment padding for the next vec4
    float nebulaColorNight[4]; // Must be 4 floats
    float nebulaColorDay[4];   // Must be 4 floats
    float auroraColor[4];      // Must be 4 floats
    float vignetteStrength;
    float overallBrightness;
    float pad1, pad2;          // Pad the end to 16-byte multiple
    float inverseView[16];
    float verticalBandFrequency;
} SkyUBO;
    extern SkyUBO skyUBOData;

    void sky_update(void);