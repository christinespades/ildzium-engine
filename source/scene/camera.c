#include <math.h>
#include <string.h>

#include "scene/camera.h"
#include "core/math.h"
#include "rendering/renderer.h"
#include "ui/ui.h"

Camera camera = {0.0f, 0.0f, 5.0f, -90.0f, 0.0f, 15.0f};

#define PI 3.14159265359f

void init_camera(void)
{
    camera.x = 0.0f;
    camera.y = 0.0f;
    camera.z = 5.0f;
    camera.yaw = -90.0f;
    camera.pitch = 0.0f;
    camera.speed = 15.0f;
}

void matrix_identity(float* out)
{
    memset(out, 0, 16 * sizeof(float));
    out[0] = out[5] = out[10] = out[15] = 1.0f;
}

void matrix_multiply(const float* a, const float* b, float* out)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            out[i*4 + j] = a[i*4 + 0] * b[0*4 + j] +
                           a[i*4 + 1] * b[1*4 + j] +
                           a[i*4 + 2] * b[2*4 + j] +
                           a[i*4 + 3] * b[3*4 + j];
        }
    }
}

// Simple look-at style view matrix from camera position + yaw/pitch
void camera_get_view_matrix(float* out_view)
{
    float yawRad   = camera.yaw   * (PI / 180.0f);
    float pitchRad = camera.pitch * (PI / 180.0f);

    float cosPitch = cosf(pitchRad);
    float sinPitch = sinf(pitchRad);
    float cosYaw   = cosf(yawRad);
    float sinYaw   = sinf(yawRad);

    // Forward vector
    float fx = cosPitch * sinYaw;
    float fy = sinPitch;
    float fz = cosPitch * cosYaw;

    // Right vector (strafe)
    float rx = cosYaw;
    float ry = 0.0f;
    float rz = -sinYaw;

    // Up vector
    float ux = -sinPitch * sinYaw;
    float uy = cosPitch;
    float uz = -sinPitch * cosYaw;

    // Build view matrix (inverse of camera transform)
    out_view[0]  = rx;   out_view[1]  = ux;   out_view[2]  = -fx;  out_view[3]  = 0.0f;
    out_view[4]  = ry;   out_view[5]  = uy;   out_view[6]  = -fy;  out_view[7]  = 0.0f;
    out_view[8]  = rz;   out_view[9]  = uz;   out_view[10] = -fz;  out_view[11] = 0.0f;

    // Translation part
    out_view[12] = -(rx * camera.x + ry * camera.y + rz * camera.z);
    out_view[13] = -(ux * camera.x + uy * camera.y + uz * camera.z);
    out_view[14] = -(-fx * camera.x - fy * camera.y - fz * camera.z);
    out_view[15] = 1.0f;
}

void camera_get_projection_matrix(float* out_proj, float aspect_ratio)
{
    float fov = 60.0f * (PI / 180.0f);   // 60 degrees
    float near = 0.1f;
    float far  = 1000.0f;

    float f = 1.0f / tanf(fov / 2.0f);

    memset(out_proj, 0, 16 * sizeof(float));

    out_proj[0]  = f / aspect_ratio;
    out_proj[5]  = f;
    out_proj[10] = (far + near) / (near - far);
    out_proj[11] = -1.0f;
    out_proj[14] = (2.0f * far * near) / (near - far);
    out_proj[15] = 0.0f;
}

// Keep your existing update_camera (unchanged)
void update_camera(float deltaTime)
{
    if (g_ui_ctx->cursor_captured) return;

    float velocity = camera.speed * deltaTime;
    float yawRad = camera.yaw * (PI / 180.0f);

    if (glfwGetKey(g_window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.x += sinf(yawRad) * velocity;
        camera.z += cosf(yawRad) * velocity;
    }
    if (glfwGetKey(g_window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.x -= sinf(yawRad) * velocity;
        camera.z -= cosf(yawRad) * velocity;
    }

    float strafeYaw = yawRad - (PI / 2.0f);
    if (glfwGetKey(g_window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.x += sinf(strafeYaw) * velocity;
        camera.z += cosf(strafeYaw) * velocity;
    }
    if (glfwGetKey(g_window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.x -= sinf(strafeYaw) * velocity;
        camera.z -= cosf(strafeYaw) * velocity;
    }

    if (glfwGetKey(g_window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.y += velocity;
    if (glfwGetKey(g_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.y -= velocity;
}


void update_camera_ubo(void)
{
    float view[16], proj[16], invView[16];

    camera_get_view_matrix(view);
    camera_get_projection_matrix(proj, (float)swapchainExtent.width / (float)swapchainExtent.height);

    if (!matrix_inverse(view, invView)) {
        // Fallback to identity if inversion fails (should rarely happen)
        matrix_identity(invView);
    }

    // Copy to camera UBO (keep your existing cameraUBOData)
    memcpy(cameraUBOData.view, view, 16 * sizeof(float));
    memcpy(cameraUBOData.proj, proj, 16 * sizeof(float));
    memcpy(cameraUBOData.inverseView, invView, 16 * sizeof(float));  // Add this field to CameraUBO

    void* data;
    vkMapMemory(device, cameraUBOMemory, 0, sizeof(CameraUBO), 0, &data);
    memcpy(data, &cameraUBOData, sizeof(CameraUBO));
    vkUnmapMemory(device, cameraUBOMemory);
}