#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

typedef struct {
    float x, y, z;
    float yaw, pitch;
    float speed;
} Camera;

extern Camera camera;
extern GLFWwindow* g_window;

typedef struct {
    float view[16];
    float proj[16];
    float inverseView[16];
} CameraUBO;
CameraUBO cameraUBOData;

VkBuffer cameraUBOBuffer;
VkDeviceMemory cameraUBOMemory;

void init_camera(void);
void update_camera(float deltaTime);
void update_camera_ubo(void);

// Matrix helpers (column-major, as expected by Vulkan/OpenGL)
void camera_get_view_matrix(float* out_view);
void camera_get_projection_matrix(float* out_proj, float aspect_ratio);
void matrix_multiply(const float* a, const float* b, float* out);   // out = a * b
void matrix_identity(float* out);