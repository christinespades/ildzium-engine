#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>

#include "camera.h"
#include "input.h"
#include "main.h"          // <-- Important: declares the functions
#include "ui.h"

// Globals
GLFWwindow* g_window = NULL;
VkInstance vk_instance = VK_NULL_HANDLE;
VkSurfaceKHR vk_surface = VK_NULL_HANDLE;

// Forward declarations for renderer
void init_renderer(VkInstance instance, VkSurfaceKHR surface);
void cleanup_renderer();
void draw_frame();

void on_button_clicked(void)
{
    printf("Button clicked!\n");
}

void init_ui(void) {
    ui_init(&ui_ctx);
    ui_add_button(&ui_ctx, 250, 250, 380, 324, "Click Me", on_button_clicked);
    ui_add_button(&ui_ctx, 750, 750, 180, 324, "Click Me As Well", on_button_clicked);
}

int main()
{
    init_glfw();
    init_ui();
    create_vulkan_instance();
    create_surface();

    init_renderer(vk_instance, vk_surface);
    init_camera();

    printf("Ildzium Engine started\n");

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(g_window))
    {
        glfwPollEvents();

        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;

        update_camera(deltaTime);
        draw_frame();
    }

    cleanup_renderer();
    vkDestroySurfaceKHR(vk_instance, vk_surface, NULL);
    vkDestroyInstance(vk_instance, NULL);
    glfwDestroyWindow(g_window);
    glfwTerminate();

    return 0;
}

// ====================== Function Definitions ======================

void init_glfw(void)
{
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Get primary monitor size
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    int screen_width = mode->width;
    int screen_height = mode->height;

    // Compute 75% of screen size
    int win_width = screen_width * 3 / 4;
    int win_height = screen_height * 3 / 4;

    // Create the window
    g_window = glfwCreateWindow(win_width, win_height, "Ildzium Engine", NULL, NULL);
    if (!g_window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        exit(1);
    }

    // Center the window
    int win_x = (screen_width - win_width) / 2;
    int win_y = (screen_height - win_height) / 2;
    glfwSetWindowPos(g_window, win_x, win_y);

    glfwSetKeyCallback(g_window, key_callback);
    glfwSetCursorPosCallback(g_window, mouse_callback);
    glfwSetScrollCallback(g_window, scroll_callback);
    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void create_vulkan_instance(void)
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    const char* layers[] = { "VK_LAYER_KHRONOS_validation" };

    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Ildzium";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Ildzium";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = layers;

    if (vkCreateInstance(&createInfo, NULL, &vk_instance) != VK_SUCCESS) {
        printf("Failed to create Vulkan instance\n");
        exit(1);
    }
}

void create_surface(void)
{
    if (glfwCreateWindowSurface(vk_instance, g_window, NULL, &vk_surface) != VK_SUCCESS) {
        printf("Failed to create window surface\n");
        exit(1);
    }
}