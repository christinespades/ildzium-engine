#pragma once

#define CAMERA_LOCATION_DEFAULT V3_BLACK
#define CAMERA_LOCATION_DEFAULT_MIN (vec3){-100000.0f, -100000.0f, -100000.0f}
#define CAMERA_LOCATION_DEFAULT_MAX (vec3){100000.0f, 100000.0f, 100000.0f}
#define CAMERA_ACCELERATION 1.0f
#define CAMERA_FAR_PLANE 1000000.0f
#define CAMERA_FOV 60.0f
#define CAMERA_NEAR_PLANE 0.2f
#define CAMERA_PITCH_DEFAULT 0.0f
#define CAMERA_SPEED_DEFAULT 15.0f
#define CAMERA_SPEED_MAX 150000.0f
#define CAMERA_YAW_DEFAULT 0.0f

#define CAMERA_PARAMS_MAP \
    X(f, CAMERA_ACCELERATION, "Camera Acceleration", 0.1f, 10.0f, tooltip_EMPTY) \
    X(f, CAMERA_FAR_PLANE, "Far Plane", 0.0f, 10000000.0f, tooltip_EMPTY) \
    X(f, CAMERA_FOV, "FOV", 0.0f, 100000.0f, tooltip_EMPTY) \
    X(f, CAMERA_NEAR_PLANE, "Near Plane", 0.0f, 10.0f, tooltip_EMPTY) \
    X(f, CAMERA_PITCH_DEFAULT, "Camera Pitch Default", -179.0f, 179.0f, tooltip_EMPTY) \
    X(f, CAMERA_SPEED_DEFAULT, "Speed (Default)", 15.0f, 1000.0f, tooltip_EMPTY) \
    X(f, CAMERA_SPEED_MAX, "Speed (Max)", 0.0f, 999999.0f, tooltip_EMPTY) \
    X(f, CAMERA_YAW_DEFAULT, "Camera YAW Default", -179.0f, 179.0f, tooltip_EMPTY)

#define CAMERA_PARAMS_V3_MAP \
    X(v3, CAMERA_LOCATION_DEFAULT, "Camera Location (Default)", CAMERA_LOCATION_DEFAULT_MIN, CAMERA_LOCATION_DEFAULT_MAX, tooltip_EMPTY)

