#include "pch.h"
#include "scene/camera.h"
#include "rendering/device.h"
#ifdef __EMSCRIPTEN__
    #include "rendering/renderer_webgpu.h"
    WGPUBuffer cameraBuffer = NULL;
#else
    #include "rendering/renderer_vk.h"
#endif

Camera camera = {0};
CameraUBO cameraUBOData = {0};

#define PI 3.14159265359f

void init_camera(void)
{
    vec3 pos = get_param_vec3(PARAM_CAMERA_LOCATION_DEFAULT);
    camera.x = pos.x;
    camera.y = pos.y;
    camera.z = pos.z;
    camera.yaw = get_param_float(PARAM_CAMERA_YAW_DEFAULT);
    camera.pitch = get_param_float(PARAM_CAMERA_PITCH_DEFAULT);
    camera.speed = get_param_float(PARAM_CAMERA_SPEED_DEFAULT);
}

// Simple look-at style view matrix from camera position + yaw/pitch
void camera_get_view_matrix(float* out_view)
{
    out_view[0]  = camera.right.x;
    out_view[1]  = camera.up.x;
    out_view[2]  = -camera.forward.x;
    out_view[3]  = 0.0f;

    out_view[4]  = camera.right.y;
    out_view[5]  = camera.up.y;
    out_view[6]  = -camera.forward.y;
    out_view[7]  = 0.0f;

    out_view[8]  = camera.right.z;
    out_view[9]  = camera.up.z;
    out_view[10] = -camera.forward.z;
    out_view[11] = 0.0f;

    out_view[12] = -(camera.right.x * camera.x +
                     camera.right.y * camera.y +
                     camera.right.z * camera.z);

    out_view[13] = -(camera.up.x * camera.x +
                     camera.up.y * camera.y +
                     camera.up.z * camera.z);

    out_view[14] = -(-camera.forward.x * camera.x -
                     camera.forward.y * camera.y -
                     camera.forward.z * camera.z);

    out_view[15] = 1.0f;
}

void camera_get_projection_matrix(float* out_proj, float aspect_ratio)
{
    float fov = get_param_float(PARAM_CAMERA_FOV) * (PI / 180.0f);   // 60 degrees
    float nearPlane = get_param_float(PARAM_CAMERA_NEAR_PLANE);
    float farPlane  = get_param_float(PARAM_CAMERA_FAR_PLANE);

    float f = 1.0f / tanf(fov / 2.0f);

    memset(out_proj, 0, 16 * sizeof(float));

    out_proj[0]  = f / aspect_ratio;
    out_proj[5]  = f;
    out_proj[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
    out_proj[11] = -1.0f;
    out_proj[14] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
    out_proj[15] = 0.0f;
}

void update_camera()
{
    if (g_ui_ctx->cursor_captured) return;

    float yawRad   = DEG2RAD(camera.yaw);
    float pitchRad = DEG2RAD(camera.pitch);

    float cosPitch = cosf(pitchRad);
    float sinPitch = sinf(pitchRad);
    float cosYaw   = cosf(yawRad);
    float sinYaw   = sinf(yawRad);

    camera.forward = (vec3){
        cosPitch * sinYaw,
        sinPitch,
        cosPitch * cosYaw
    };

    camera.right = (vec3){
        cosYaw,
        0.0f,
        -sinYaw
    };

    camera.up = vec3_cross(camera.forward, camera.right);
    camera.velocity = camera.speed * g_dt;
}

void update_camera_ubo(void)
{
    float view[16], proj[16], invView[16];

    camera_get_view_matrix(view);
    float aspect = (float)g_width / (float)g_height;
    // printf("Camera UBO update - pos: (%.2f, %.2f, %.2f)  yaw: %.1f  pitch: %.1f  aspect: %.3f\n", camera.x, camera.y, camera.z, camera.yaw, camera.pitch, aspect);
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
    cameraUBOData.viewport[0] = g_width;
    cameraUBOData.viewport[1] = g_height;
    void* data;
    vkMapMemory(vk_device, cameraUBOMemory, 0, sizeof(CameraUBO), 0, &data);
    memcpy(data, &cameraUBOData, sizeof(CameraUBO));
    vkUnmapMemory(vk_device, cameraUBOMemory);
#endif
}