#pragma once
#include "core/math.h"
#include "input/input.h"
#include "rendering/renderer.h"
#include "rendering/surface.h"
#include "ui/ui.h"

typedef struct {
    float x, y, z;
    float yaw, pitch;
    float speed;
} Camera;

extern Camera camera;

#ifndef __EMSCRIPTEN__
    extern GLFWwindow* g_window;
    VkBuffer cameraUBOBuffer;
    VkDeviceMemory cameraUBOMemory;
#else
    WGPUBuffer cameraBuffer;
#endif

typedef struct {
    float view[16];
    float proj[16];
    float inverseView[16];
} CameraUBO;
CameraUBO cameraUBOData;

void init_camera(void);
void update_camera(float deltaTime);
void update_camera_ubo(void);

// Matrix helpers (column-major, as expected by Vulkan/OpenGL)
void camera_get_view_matrix(float* out_view);
void camera_get_projection_matrix(float* out_proj, float aspect_ratio);
