#pragma once
#include "core/memory.h"
#include "scene/model.h"
#include <stdint.h>
#include <string.h>

// 16-byte aligned vec3 to perfectly match GLSL std140 (prevents alignment bugs)
typedef struct {
    float x, y, z;
    float _pad;
} Vec3Std140;

// Lights exactly matching the shader UBO
typedef struct {
    Vec3Std140 direction;
    Vec3Std140 color;
    float intensity;
    float _pad[3];           // keep struct size multiple of 16
} DirectionalLightStd140;

typedef struct {
    Vec3Std140 position;
    Vec3Std140 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    // total = 48 bytes → perfect multiple of 16
} PointLightStd140;

typedef struct {
    Vec3Std140 ambient;                    // ambient strength
    DirectionalLightStd140 dirLight;
    int32_t numPointLights;                // 0..4
    float _pad1[3];                        // alignment padding
    PointLightStd140 pointLights[4];
    Vec3Std140 viewPos;                    // camera world position
} LightingUBO;                             // total size ≈ 16 + 48 + 16 + 192 + 16 = 288 bytes

typedef struct {
    VkBuffer buffer;
    VkDeviceMemory memory;
    void* mapped;              // persistent mapped pointer
    VkDescriptorSet descriptorSet;
} LightsSystem;

LightsSystem g_lights;
VkDescriptorSetLayout lightDescriptorSetLayout;

void init_lights(VkDevice device,
                 VkPhysicalDevice phys);
void init_lights_write(VkDevice device);
void lights_update(VkCommandBuffer cmdBuf,
                   VkPipelineLayout pipelineLayout,
                   float ambientR, float ambientG, float ambientB,
                   float dirX, float dirY, float dirZ,
                   float dirIntensity,
                   int numPoints,
                   float camX, float camY, float camZ);