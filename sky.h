#pragma once
#include <vulkan/vulkan.h>

// Exposed tunable parameters (change these at runtime if you want)
typedef struct {
    // Time & cycle
    float timeOfDay;        // 0.0 = midnight, 0.25 = sunrise, 0.5 = noon, 0.75 = sunset, 1.0 = midnight
    float cycleSpeed;       // multiplier for automatic progression (set to 0 to disable auto cycle)

    // Nebula
    float nebulaSpeed;           // overall nebula animation speed multiplier
    float nebulaScale;           // base scale of noise layers
    float nebulaIntensity;       // global nebula brightness
    float nebulaLayerCount;      // how many layers (int cast in shader, but float here for ease)

    // Colors (nebula layers interpolate between these)
    float nebulaColor1[3];       // vec3
    float nebulaColor2[3];

    // Stars
    float starCount;             // approx number of stars (shader loops to this)
    float starBrightness;        // base brightness
    float starTwinkleSpeed;      // how fast stars twinkle
    float starSize;              // how "big" the star points are (smoothstep radius)

    // Aurora
    float auroraIntensity;
    float auroraSpeed;
    float auroraColor[3];

    // Day/Night blending control
    float dayNightBlend;    // 0 = full night, 1 = full day (computed from timeOfDay, but overridable)

    // General
    float vignetteStrength;
    float overallBrightness;
} SkyParameters;

extern SkyParameters g_skyParams;   // global, you can tweak this anywhere

// Vulkan UBO for the sky (contains all dynamic parameters + camera rotation)
typedef struct {
    float time;             // absolute animated time
    float yaw;              // camera yaw in radians
    float pitch;            // camera pitch in radians
    float timeOfDay;        // 0..1 cycle
    float dayNightBlend;    // 0 = night, 1 = day

    // Nebula
    float nebulaScale;
    float nebulaIntensity;
    float nebulaLayerCount;

    // Stars
    float starCount;
    float starBrightness;
    float starTwinkleSpeed;
    float starSize;

    // Aurora
    float auroraIntensity;
    float auroraSpeed;

    // Colors (day and night versions)
    float nebulaColorNight[3];
    float nebulaColorDay[3];
    float auroraColor[3];

    float vignetteStrength;
    float overallBrightness;
    float pad[2];           // alignment
} SkyUBO;

extern VkPipeline skyPipeline;
extern VkPipelineLayout skyPipelineLayout;
extern VkShaderModule vertShaderModule;
extern VkShaderModule fragShaderModule;

void sky_init(void);
void sky_cleanup(void);
void sky_draw(VkCommandBuffer cmd);          // call this instead of the old inline sky code
void sky_update(void);                  // call every frame before drawing

// Helpers
void sky_set_time_of_day(float tod);    // 0.0 = midnight ... 0.5 = noon
void sky_set_cycle_speed(float speed);  // 0 = manual control only
void sky_set_nebula_colors(const float col1[3], const float col2[3]);