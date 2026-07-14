#include "pch.h"
#ifdef __EMSCRIPTEN__
    #include "core/window/window_webgpu.h"

    extern WGPUDevice device;
    extern WGPUTextureFormat swapchainFormat;
    extern WGPUSurface surface;

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

    void init_window(void)
    {
        emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, on_resize);
        on_resize(0, NULL, NULL);
    }

    void ildz_get_window_size(int* w, int* h)
    {
        *w = g_width;
        *h = g_height;
    }
#endif