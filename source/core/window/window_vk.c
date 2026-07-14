#include "pch.h"
#ifndef __EMSCRIPTEN__
    #include "core/window/window_vk.h"
    #define STB_IMAGE_IMPLEMENTATION
	#include "stb_image.h"
    GLFWwindow* g_window = NULL;
	static int framebufferResized = 0;

	static void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
	{
	    framebufferResized = 1;
	    g_width = width;
	    g_height = height;
	}

    void init_window(void)
    {
        if (!glfwInit()) {
            LOGE("Failed to initialize GLFW");
            exit(1);
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        if (get_param_bool(ENGINE_WINDOW_BORDERLESS))
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(primary);
        int screen_width = mode->width;
        int screen_height = mode->height;

        // Compute N% of screen size
        int win_width = screen_width * get_param_color(PARAM_ENGINE_WINDOW_SIZE_PERCENTAGE_OF_MONITOR) / 100;
        int win_height = screen_height * get_param_color(PARAM_ENGINE_WINDOW_SIZE_PERCENTAGE_OF_MONITOR) / 100;

        g_window = glfwCreateWindow(win_width, win_height, "Ildzium Engine", NULL, NULL);
        if (!g_window) {
            LOGE("Failed to create GLFW window");
            glfwTerminate();
            exit(1);
        }

        // framebuffer size, not window size, important for DPI
        glfwSetFramebufferSizeCallback(g_window, framebuffer_resize_callback);

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
    }

    int ildz_should_window_close()
    {
        return glfwWindowShouldClose(g_window);
    }

    void ildz_get_window_size(int* w, int* h)
    {
        glfwGetWindowSize(g_window, w, h);
    }
#endif