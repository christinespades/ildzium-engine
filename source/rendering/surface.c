#include "surface.h"
#include <stdio.h>    // FILE, fopen, fread, fseek, ftell, fclose, printf
#include <stdlib.h>   // malloc, free, exit, NULL
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "input/input.h"

GLFWwindow* g_window = NULL;
VkInstance vk_instance = VK_NULL_HANDLE;
VkSurfaceKHR vk_surface = VK_NULL_HANDLE;

void surface_init(void)
{
	init_glfw();
	create_vulkan_instance();
	create_surface();
}

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

    // Add icon
    int width, height, channels;
    unsigned char* pixels = stbi_load("../../icons/main.png", &width, &height, &channels, 4);

    if (pixels) {
        GLFWimage icon;
        icon.width = width;
        icon.height = height;
        icon.pixels = pixels;

        glfwSetWindowIcon(g_window, 1, &icon);
        stbi_image_free(pixels);
    } else {
        printf("Failed to load icon\n");
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