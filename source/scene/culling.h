#pragma once
#include "scene/camera.h"
#include "core/params/params.h"

typedef struct { float normal[3]; float distance; } Plane;

extern int g_width;
extern int g_height;
extern Camera camera;

// Extracts frustum planes from a column-major View-Projection Matrix
inline void extract_frustum_planes(const float* vp, Plane planes[6]) {
    // Left Plane
    planes[0].normal[0] = vp[3]  + vp[0];  planes[0].normal[1] = vp[7]  + vp[4];
    planes[0].normal[2] = vp[11] + vp[8];  planes[0].distance  = vp[15] + vp[12];
    // Right Plane
    planes[1].normal[0] = vp[3]  - vp[0];  planes[1].normal[1] = vp[7]  - vp[4];
    planes[1].normal[2] = vp[11] - vp[8];  planes[1].distance  = vp[15] - vp[12];
    // Bottom Plane
    planes[2].normal[0] = vp[3]  + vp[1];  planes[2].normal[1] = vp[7]  + vp[5];
    planes[2].normal[2] = vp[11] + vp[9];  planes[2].distance  = vp[15] + vp[13];
    // Top Plane
    planes[3].normal[0] = vp[3]  - vp[1];  planes[3].normal[1] = vp[7]  - vp[5];
    planes[3].normal[2] = vp[11] - vp[9];  planes[3].distance  = vp[15] - vp[13];
    // Near Plane
    planes[4].normal[0] = vp[2];           planes[4].normal[1] = vp[6];
    planes[4].normal[2] = vp[10];          planes[4].distance  = vp[14];
    // Far Plane
    planes[5].normal[0] = vp[3]  - vp[2];  planes[5].normal[1] = vp[7]  - vp[6];
    planes[5].normal[2] = vp[11] - vp[10]; planes[5].distance  = vp[15] - vp[14];

    // Normalize planes
    for (int i = 0; i < 6; i++) {
        float length = sqrtf(planes[i].normal[0] * planes[i].normal[0] +
                             planes[i].normal[1] * planes[i].normal[1] +
                             planes[i].normal[2] * planes[i].normal[2]);
        planes[i].normal[0] /= length;
        planes[i].normal[1] /= length;
        planes[i].normal[2] /= length;
        planes[i].distance  /= length;
    }
}

// Geometric frustum extraction based directly on camera position and direction
inline void extract_frustum_geometric(Plane planes[6]) {
    #define DEG_TO_RAD (3.14159265359f / 180.0f)
    float yawRad   = camera.yaw   * DEG_TO_RAD;
    float pitchRad = camera.pitch * DEG_TO_RAD;

    float cosPitch = cosf(pitchRad);
    float sinPitch = sinf(pitchRad);
    float cosYaw   = cosf(yawRad);
    float sinYaw   = sinf(yawRad);

    // Reconstruct the exact direction vectors your camera uses
    float front[3] = { cosPitch * sinYaw, sinPitch, cosPitch * cosYaw };
    float right[3] = { cosYaw, 0.0f, -sinYaw };
    float up[3]    = { -sinPitch * sinYaw, cosPitch, -sinPitch * cosYaw };

    float fov = get_param_float(PARAM_CAMERA_FOV) * DEG_TO_RAD;
    float nearPlane = get_param_float(PARAM_CAMERA_NEAR_PLANE);
    float farPlane  = get_param_float(PARAM_CAMERA_FAR_PLANE);

    float aspect = (float)g_width / (float)g_height;

    // Compute dimensions of near and far planes
    float tang = tanf(fov * 0.5f);
    float nh = nearPlane * tang;
    float nw = nh * aspect;
    // float fh = farPlane * tang;
    // float fw = fh * aspect;

    // Camera Position
    float camPos[3] = { camera.x, camera.y, camera.z };

    // Near and Far Centers
    float nc[3], fc[3];
    for(int i=0; i<3; i++) {
        nc[i] = camPos[i] + front[i] * nearPlane;
        fc[i] = camPos[i] + front[i] * farPlane;
    }

    // Helper macro to build plane from Normal and Point
    #define SET_PLANE(p, nx, ny, nz, pt) { \
        p.normal[0] = nx; p.normal[1] = ny; p.normal[2] = nz; \
        float len = sqrtf(p.normal[0]*p.normal[0] + p.normal[1]*p.normal[1] + p.normal[2]*p.normal[2]); \
        p.normal[0]/=len; p.normal[1]/=len; p.normal[2]/=len; \
        p.distance = -(p.normal[0]*pt[0] + p.normal[1]*pt[1] + p.normal[2]*pt[2]); \
    }

    // 1. Near Plane
    SET_PLANE(planes[4], -front[0], -front[1], -front[2], nc);
    // 2. Far Plane
    SET_PLANE(planes[5], front[0], front[1], front[2], fc);

    // Aux vectors for sides
    float aux[3], normal[3], point[3];

    // 3. Top Plane
    for(int i=0; i<3; i++) point[i] = nc[i] + up[i] * nh;
    // aux = (point - camPos) cross right
    aux[0] = point[0] - camPos[0]; aux[1] = point[1] - camPos[1]; aux[2] = point[2] - camPos[2];
    normal[0] = aux[1]*right[2] - aux[2]*right[1];
    normal[1] = aux[2]*right[0] - aux[0]*right[2];
    normal[2] = aux[0]*right[1] - aux[1]*right[0];
    SET_PLANE(planes[3], normal[0], normal[1], normal[2], point);

    // 4. Bottom Plane
    for(int i=0; i<3; i++) point[i] = nc[i] - up[i] * nh;
    aux[0] = point[0] - camPos[0]; aux[1] = point[1] - camPos[1]; aux[2] = point[2] - camPos[2];
    normal[0] = right[1]*aux[2] - right[2]*aux[1];
    normal[1] = right[2]*aux[0] - right[0]*aux[2];
    normal[2] = right[0]*aux[1] - right[1]*aux[0];
    SET_PLANE(planes[2], normal[0], normal[1], normal[2], point);

    // 5. Left Plane
    for(int i=0; i<3; i++) point[i] = nc[i] - right[i] * nw;
    aux[0] = point[0] - camPos[0]; aux[1] = point[1] - camPos[1]; aux[2] = point[2] - camPos[2];
    normal[0] = aux[1]*up[2] - aux[2]*up[1];
    normal[1] = aux[2]*up[0] - aux[0]*up[2];
    normal[2] = aux[0]*up[1] - aux[1]*up[0];
    SET_PLANE(planes[0], normal[0], normal[1], normal[2], point);

    // 6. Right Plane
    for(int i=0; i<3; i++) point[i] = nc[i] + right[i] * nw;
    aux[0] = point[0] - camPos[0]; aux[1] = point[1] - camPos[1]; aux[2] = point[2] - camPos[2];
    normal[0] = up[1]*aux[2] - up[2]*aux[1];
    normal[1] = up[2]*aux[0] - up[0]*aux[2];
    normal[2] = up[0]*aux[1] - up[1]*aux[0];
    SET_PLANE(planes[1], normal[0], normal[1], normal[2], point);
}

inline bool is_visible(const Plane planes[6], const float center[3], float radius) {
    if (radius <= 0.0f) return true; // Skip if no bounding box provided
    for (int i = 0; i < 6; i++) {
        float distance = planes[i].normal[0] * center[0] +
                         planes[i].normal[1] * center[1] +
                         planes[i].normal[2] * center[2] +
                         planes[i].distance;
        if (distance < -radius) return false; // Entirely behind plane
    }
    return true;
}

// Helper to transform a 3D point by a 4x4 matrix (matching your engine's layout)
inline void transform_point(const float mat[16], const float vec[3], float out_vec[4]) {
    // This matches the layout where translation is at 12, 13, 14
    out_vec[0] = mat[0]*vec[0] + mat[4]*vec[1] + mat[8]*vec[2] + mat[12];
    out_vec[1] = mat[1]*vec[0] + mat[5]*vec[1] + mat[9]*vec[2] + mat[13];
    out_vec[2] = mat[2]*vec[0] + mat[6]*vec[1] + mat[10]*vec[2] + mat[14];
    out_vec[3] = mat[3]*vec[0] + mat[7]*vec[1] + mat[11]*vec[2] + mat[15];
}

// Helper function to safely transform a point by a column-major matrix
inline void transform_point_v2(const float mat[16], const float vec[3], float out_vec[4]) {
    out_vec[0] = mat[0]*vec[0] + mat[4]*vec[1] + mat[8]*vec[2] + mat[12];
    out_vec[1] = mat[1]*vec[0] + mat[5]*vec[1] + mat[9]*vec[2] + mat[13];
    out_vec[2] = mat[2]*vec[0] + mat[6]*vec[1] + mat[10]*vec[2] + mat[14];
    out_vec[3] = mat[3]*vec[0] + mat[7]*vec[1] + mat[11]*vec[2] + mat[15];
}

inline bool is_visible_clip_space(const float center[3], float radius) {
    float view[16], proj[16];
    camera_get_view_matrix(view);
    
    float aspect = (float)g_width / (float)g_height;
    camera_get_projection_matrix(proj, aspect);

    // 1. Transform World Space -> View Space
    float view_pos[4];
    transform_point_v2(view, center, view_pos);

    // 2. Transform View Space -> Clip Space (gl_Position equivalent)
    float clip_pos[4];
    // We treat view_pos as a vec3 for the next matrix transformation step
    transform_point_v2(proj, view_pos, clip_pos);

    float w = clip_pos[3];

    // Guard against objects exactly on or behind the camera plane to prevent division by zero anomalies
    if (w <= 0.0f) {
        // If the object's boundary can potentially cross into the positive visibility zone, keep it
        if (view_pos[2] + radius > 0.1f) {
            return true; 
        }
        return false;
    }

    // 3. Test against Vulkan standard clip space frustum boundaries expanded by the radius
    // In Clip Space, an object is visible if:
    // -w <= x <= w
    // -w <= y <= w
    //  0 <= z <= w  (Vulkan depth standard)
    
    if (clip_pos[0] < -w - radius || clip_pos[0] > w + radius) return false; // Left/Right frustum exit
    if (clip_pos[1] < -w - radius || clip_pos[1] > w + radius) return false; // Top/Bottom frustum exit
    if (clip_pos[2] < 0.0f - radius || clip_pos[2] > w + radius) return false;  // Near/Far frustum exit

    return true;
}