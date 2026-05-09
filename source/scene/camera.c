#include "pch.h"
#include "scene/camera.h"
#include "rendering/device.h"
#ifdef __EMSCRIPTEN__
    #include "rendering/renderer_webgpu.h"

WGPUBuffer cameraBuffer = NULL;
#else
    #include "rendering/renderer_vulkan.h"
#endif

Camera camera = {0.0f, 0.0f, 5.0f, -90.0f, 0.0f, 15.0f};
CameraUBO cameraUBOData = {0};

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
    float nearPlane = 0.1f;
    float farPlane  = 1000.0f;

    float f = 1.0f / tanf(fov / 2.0f);

    memset(out_proj, 0, 16 * sizeof(float));

    out_proj[0]  = f / aspect_ratio;
    out_proj[5]  = f;
    out_proj[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
    out_proj[11] = -1.0f;
    out_proj[14] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
    out_proj[15] = 0.0f;
}

void update_camera(float deltaTime)
{
    if (g_ui_ctx->cursor_captured) return;

    float velocity = camera.speed * deltaTime;
    float yawRad = camera.yaw * (PI / 180.0f);

    if (platform_get_key_down(KEY_W)) {
        camera.x += sinf(yawRad) * velocity;
        camera.z += cosf(yawRad) * velocity;
    }
    if (platform_get_key_down(KEY_S)) {
        camera.x -= sinf(yawRad) * velocity;
        camera.z -= cosf(yawRad) * velocity;
    }

    float strafeYaw = yawRad - (PI / 2.0f);

    if (platform_get_key_down(KEY_A)) {
        camera.x += sinf(strafeYaw) * velocity;
        camera.z += cosf(strafeYaw) * velocity;
    }
    if (platform_get_key_down(KEY_D)) {
        camera.x -= sinf(strafeYaw) * velocity;
        camera.z -= cosf(strafeYaw) * velocity;
    }

    // Vertical movement
    if (platform_get_key_down(KEY_SPACE)) {
        camera.y += velocity;
    }
    if (platform_get_key_down(KEY_LEFT_SHIFT)) {
        camera.y -= velocity;
    }
}

void update_camera_ubo(void)
{
    float view[16], proj[16], invView[16];

    camera_get_view_matrix(view);
#ifdef __EMSCRIPTEN__
    float aspect = (float)g_width / (float)g_height;
    //printf("Camera UBO update - pos: (%.2f, %.2f, %.2f)  yaw: %.1f  pitch: %.1f  aspect: %.3f\n", camera.x, camera.y, camera.z, camera.yaw, camera.pitch, aspect);
#else
    float aspect = (float)swapchainExtent.width / (float)swapchainExtent.height;
#endif
    camera_get_projection_matrix(proj, aspect);

#ifdef __EMSCRIPTEN__
    // 1. Flip Y for WebGPU (Vulkan is Y-down, WebGPU is Y-up)
    proj[5] *= -1.0f;

    if (!matrix_inverse(view, invView)) {
        // Fallback to identity if inversion fails (should rarely happen)
        matrix_identity(invView);
    }

    if (cameraBuffer) {
            // We need a temp struct to hold the TRANSPOSED matrices for WebGPU
            CameraUBO transposedUBO;
            
            // Helper to transpose into the new struct
            for (int r = 0; r < 4; r++) {
                for (int c = 0; c < 4; c++) {
                    transposedUBO.view[c * 4 + r] = view[r * 4 + c];
                    transposedUBO.proj[c * 4 + r] = proj[r * 4 + c];
                    transposedUBO.inverseView[c * 4 + r] = invView[r * 4 + c];
                }
            }

            // Upload the WHOLE struct (192 bytes)
            wgpuQueueWriteBuffer(queue, cameraBuffer, 0, &transposedUBO, sizeof(CameraUBO));
        }
#else
    if (!matrix_inverse(view, invView)) {
        // Fallback to identity if inversion fails (should rarely happen)
        matrix_identity(invView);
    }

    memcpy(cameraUBOData.view, view, 16 * sizeof(float));
    memcpy(cameraUBOData.proj, proj, 16 * sizeof(float));
    memcpy(cameraUBOData.inverseView, invView, 16 * sizeof(float));

    void* data;
    vkMapMemory(vk_device, cameraUBOMemory, 0, sizeof(CameraUBO), 0, &data);
    memcpy(data, &cameraUBOData, sizeof(CameraUBO));
    vkUnmapMemory(vk_device, cameraUBOMemory);
#endif
}