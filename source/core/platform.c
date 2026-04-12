#include "pch.h"
#include "platform.h"
#include "input/input.h"
#include "rendering/surface.h"
#include "rendering/renderer.h"
#include "ui/ui.h"

// TODO: how to handle window resize on Web (using emscripten_set_resize_callback) and update g_width/g_height
	void init_platform() {
	    surface_init();
	    init_input();

	    g_ui_ctx = malloc(sizeof(UI_Context));
	    ui_init(g_ui_ctx);

	#ifdef __EMSCRIPTEN__
    	webgpu_init();
	#else
	    vulkan_init();
	#endif
	}

#ifdef __EMSCRIPTEN__
	/* exists in pch.h:
	#include <emscripten.h>
	#include <emscripten/html5.h>
	*/

	static double g_time = 0.0;
	
	double platform_get_time()
	{
	    return emscripten_get_now() / 1000.0;
	}

	void platform_get_window_size(int* w, int* h)
	{
	    *w = g_width;
	    *h = g_height;
	}
#else
	extern GLFWwindow* g_window;
	extern VkInstance vk_instance;
	extern VkSurfaceKHR vk_surface;

	double platform_get_time() {
	    return glfwGetTime();
	}

	void platform_get_window_size(int* w, int* h) {
		glfwGetWindowSize(g_window, w, h);
	}

	int platform_should_close() {
	    return glfwWindowShouldClose(g_window);
	}

	void platform_shutdown() {
		vulkan_shutdown();
	    vkDestroySurfaceKHR(vk_instance, vk_surface, NULL);
	    vkDestroyInstance(vk_instance, NULL);
	    glfwDestroyWindow(g_window);
	    glfwTerminate();
	}
#endif