#include "pch.h"
#include "core/window.h"

#ifndef __EMSCRIPTEN__
    #define STB_IMAGE_IMPLEMENTATION
	#include "stb_image.h"
    GLFWwindow* g_window = NULL;
#else
    extern WGPUDevice device;
    extern WGPUTextureFormat swapchainFormat;
    extern WGPUSurface surface;
#endif
    int g_width = 1280;
    int g_height = 720;

#ifdef __EMSCRIPTEN__
static EM_BOOL on_resize(int eventType, const EmscriptenUiEvent* e, void* userData)
{
    double css_w, css_h;
    emscripten_get_element_css_size("#canvas", &css_w, &css_h);
    double dpi = emscripten_get_device_pixel_ratio();
    
    int new_width  = (int)(css_w * dpi);
    int new_height = (int)(css_h * dpi);

    if (new_width == g_width && new_height == g_height) 
        return EM_TRUE;

    g_width = new_width;
    g_height = new_height;

    // Update canvas backing store
    emscripten_set_canvas_element_size("#canvas", g_width, g_height);

    // Only reconfigure if device + surface are ready
    if (device && surface && gpu_state == GPU_STATE_READY) {
        wgpuSurfaceUnconfigure(surface);   // Important!

        WGPUSurfaceConfiguration config = {0};
        config.device = device;
        config.format = swapchainFormat;
        config.usage = WGPUTextureUsage_RenderAttachment;
        config.width = g_width;
        config.height = g_height;
        config.presentMode = WGPUPresentMode_Fifo;
        config.alphaMode = WGPUCompositeAlphaMode_Opaque;
        config.viewFormatCount = 0;
        config.viewFormats = NULL;

        wgpuSurfaceConfigure(surface, &config);
        printf("Surface reconfigured to %d x %d\n", g_width, g_height);
    }

    return EM_TRUE;
}

#else
	static int framebufferResized = 0;

	static void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
	{
	    framebufferResized = 1;
	    g_width = width;
	    g_height = height;
	}
#endif

void init_window(void)
{
#ifdef __EMSCRIPTEN__
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, on_resize);
    on_resize(0, NULL, NULL);
#else
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    int screen_width = mode->width;
    int screen_height = mode->height;

    // Compute 75% of screen size
    int win_width = screen_width * 3 / 4;
    int win_height = screen_height * 3 / 4;

    g_window = glfwCreateWindow(win_width, win_height, "Ildzium Engine", NULL, NULL);
    if (!g_window) {
        printf("Failed to create GLFW window\n");
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
#endif
}

#ifndef __EMSCRIPTEN__
	int ildz_should_close()
	{
	    return glfwWindowShouldClose(g_window);
	}
#endif

void ildz_get_window_size(int* w, int* h)
{
#ifdef __EMSCRIPTEN__
    *w = g_width;
    *h = g_height;
#else
    glfwGetWindowSize(g_window, w, h);
#endif
}