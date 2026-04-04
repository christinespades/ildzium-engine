#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>

#include "camera.h"
#include "input.h"
#include "main.h"
#include "ui.h"
#include "model.h"
#include "surface.h"
#include "watcher.h"

static int g_target_fps = 60;

// Forward declarations for renderer
void init_renderer(VkInstance instance, VkSurfaceKHR surface);
void cleanup_renderer();
void draw_frame();
extern GLFWwindow* g_window;
extern VkInstance vk_instance;
extern VkSurfaceKHR vk_surface;

int main()
{
    surface_init();
    ui_init(&ui_ctx);
    init_renderer(vk_instance, vk_surface);
    init_model_system();
    load_model("../../meshes/cs_goddess_statue_opt.glb");
    // Add many instances
    for (int i = 0; i < 200; i++) {
        float transform[16] = {0}; // fill with your TRS matrix
        matrix_identity(transform);
        transform[12] = (float)(i % 20) * 2.0f - 20.0f;
        transform[13] = 0.0f;
        transform[14] = (float)(i / 20) * 2.0f - 10.0f;

        float color[4] = {0.2f + (i%5)*0.2f, 0.6f, 0.8f, 1.0f};
        add_model_instance(transform, color);
    }
    watcher_init();
    init_camera();

    double lastTime = glfwGetTime();
    double last_render_time = glfwGetTime();
    double frame_duration = 1.0 / (double)g_target_fps;

    while (!glfwWindowShouldClose(g_window))
    {
        glfwPollEvents();
        
        watcher_update();

        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;

        update_camera(deltaTime);

        // ==================== UI INPUT HANDLING ====================
        int left_pressed = glfwGetMouseButton(g_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        if (ui_ctx.cursor_captured) {
            // When UI is active (cursor visible), use absolute cursor position
            double mx, my;
            glfwGetCursorPos(g_window, &mx, &my);
            
            // Optional: clamp to window size to be safe
            int win_w, win_h;
            glfwGetWindowSize(g_window, &win_w, &win_h);
            if (mx < 0) mx = 0;
            if (my < 0) my = 0;
            if (mx >= win_w) mx = win_w - 1;
            if (my >= win_h) my = win_h - 1;

            ui_update(&ui_ctx, (int)mx, (int)my, left_pressed, deltaTime);
        }
        else {
            // Game mode - mouse movement already handled in callback
            // You can still poll left click here if you want camera actions
        }
        // ===========================================================
        currentTime = glfwGetTime();

        if ((currentTime - last_render_time) >= frame_duration)
        {
            last_render_time = currentTime;
            draw_frame();
        }
    }

    watcher_cleanup();
    cleanup_renderer();
    vkDestroySurfaceKHR(vk_instance, vk_surface, NULL);
    vkDestroyInstance(vk_instance, NULL);
    glfwDestroyWindow(g_window);
    ui_cleanup(&ui_ctx);
    glfwTerminate();

    return 0;
}