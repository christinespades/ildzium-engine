#pragma once
#include <GLFW/glfw3.h>

typedef struct {
    float x, y, z;
    float yaw, pitch;
    float speed;
} Camera;

extern Camera camera;
extern GLFWwindow* g_window;

void init_camera(void);
void update_camera(float deltaTime);