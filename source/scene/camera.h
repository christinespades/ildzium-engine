#pragma once
#include "core/params/params.h"

typedef struct {
    float x, y, z;
    float yaw, pitch;
    float speed;
    vec3 forward;
    vec3 right;
    vec3 up;
    float velocity;
} Camera;

extern Camera camera;

// Matrix helpers (column-major, as expected by Vulkan/OpenGL)
void camera_get_view_matrix(float* out_view);
void camera_get_projection_matrix(float* out_proj, float aspect_ratio);

#include "input/input.h"
#include "rendering/fps.h"
#include "rendering/renderer.h"
#include "rendering/surface.h"
#include "ui/ui.h"

#ifndef __EMSCRIPTEN__
    extern GLFWwindow* g_window;
    VkBuffer cameraUBOBuffer;
    VkDeviceMemory cameraUBOMemory;
#else
    extern WGPUBuffer cameraBuffer;
#endif

typedef struct {
    float view[16];
    float proj[16];
    float inverseView[16];
    float viewport[2];
    float padding[2];
} CameraUBO;

extern CameraUBO cameraUBOData;

void init_camera(void);
void update_camera();
void update_camera_ubo(void);