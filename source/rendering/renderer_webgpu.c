#include "pch.h"
#ifdef __EMSCRIPTEN__
    #include "renderer_webgpu.h"

    extern CameraUBO cameraUBOData;
    extern int g_width;
    extern int g_height;

    WGPUInstance instance = NULL;
    WGPUAdapter globalAdapter = NULL;
    WGPUDevice device = NULL;
    extern WGPUQueue queue;
    WGPUSurface surface = NULL;
    WGPUTextureFormat swapchainFormat = WGPUTextureFormat_BGRA8Unorm;

    WGPUTexture depthTexture = NULL;
    WGPUTextureView depthView = NULL;
    int currentDepthWidth = 0;
    int currentDepthHeight = 0;

    GPUState gpu_state = GPU_STATE_NOT_READY;

    extern WGPUBuffer cameraBuffer;

    void my_error_callback(WGPUErrorType type, const char* message, void* userdata)
    {
        const char* typeStr = "Unknown";
        switch (type) {
            case WGPUErrorType_NoError:     typeStr = "NoError"; break;
            case WGPUErrorType_Validation:  typeStr = "Validation"; break;
            case WGPUErrorType_OutOfMemory: typeStr = "OutOfMemory"; break;
            case WGPUErrorType_Internal:    typeStr = "Internal"; break;
            case WGPUErrorType_Unknown:     typeStr = "Unknown"; break;
            // WGPUErrorType_DeviceLost may not exist in your webgpu.h yet
            // case WGPUErrorType_DeviceLost:  typeStr = "DeviceLost"; break;
            default:                        typeStr = "Other"; break;
        }

        printf("WebGPU Uncaptured Error (%s): %s\n", typeStr, message ? message : "(no message)");

        // Optional: Also print to browser console via EM_ASM
        EM_ASM({
            console.error("WebGPU Error:", UTF8ToString($0), " - ", UTF8ToString($1));
        }, typeStr, message ? message : "(no message)");
    }

    static void onAdapterRequest(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void* userdata2);
    static void onDeviceRequest(WGPURequestDeviceStatus status, WGPUDevice dev, WGPUStringView message, void* userdata1, void* userdata2);

    extern void ensure_depth_texture(void);

    void webgpu_init(void)
    {
        printf("Initializing WebGPU (Emdawnwebgpu)...\n");

        instance = wgpuCreateInstance(NULL);
        if (!instance) {
            printf("Failed to create instance\n");
            return;
        }

        // Surface from canvas
        WGPUEmscriptenSurfaceSourceCanvasHTMLSelector canvasSource = {0};
        canvasSource.chain.sType = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector;
        canvasSource.selector = (WGPUStringView){ .data = "#canvas", .length = WGPU_STRLEN };

        WGPUSurfaceDescriptor surfaceDesc = {0};
        surfaceDesc.nextInChain = &canvasSource.chain;
        surface = wgpuInstanceCreateSurface(instance, &surfaceDesc);
        if (!surface) {
            printf("Failed to create surface\n");
            return;
        }

        WGPURequestAdapterOptions adapterOpts = {0};
        adapterOpts.compatibleSurface = surface;

        WGPURequestAdapterCallbackInfo adapterInfo = {0};
        adapterInfo.mode = WGPUCallbackMode_AllowProcessEvents;
        adapterInfo.callback = onAdapterRequest;

        wgpuInstanceRequestAdapter(instance, &adapterOpts, adapterInfo);

        // Wait for async initialization
        while (gpu_state != GPU_STATE_READY) {
            wgpuInstanceProcessEvents(instance);
            emscripten_sleep(10);
        }

        printf("WebGPU initialization completed successfully\n");
    }

    // Called from onDeviceRequest
    static void finish_device_setup(void)
    {
        queue = wgpuDeviceGetQueue(device);

        // Surface configure
        WGPUSurfaceCapabilities caps = {0};
        wgpuSurfaceGetCapabilities(surface, globalAdapter, &caps);
        swapchainFormat = caps.formats[0];

        WGPUSurfaceConfiguration config = {0};
        config.device = device;
        config.format = swapchainFormat;
        config.usage = WGPUTextureUsage_RenderAttachment;
        config.width = (uint32_t)g_width;
        config.height = (uint32_t)g_height;
        config.presentMode = WGPUPresentMode_Fifo;
        config.alphaMode = WGPUCompositeAlphaMode_Opaque;
        config.viewFormatCount = 0;
        config.viewFormats = NULL;

        wgpuSurfaceConfigure(surface, &config);
        printf("Surface configured: %d x %d\n", g_width, g_height);

        // Camera UBO
        WGPUBufferDescriptor bufDesc = {0};
        bufDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
        bufDesc.size = sizeof(CameraUBO);
        cameraBuffer = wgpuDeviceCreateBuffer(device, &bufDesc);

        // Create depth texture
        ensure_depth_texture();

        // Initialize your actual rendering systems
        create_sky_pipeline_webgpu();
        init_lights_webgpu(device);
        init_model_system();
        ui_renderer_init();

        gpu_state = GPU_STATE_READY;
        printf("✅ WebGPU fully ready (Sky + Models + UI)\n");
    }

    static void onDeviceRequest(WGPURequestDeviceStatus status, WGPUDevice dev, WGPUStringView message, void* userdata1, void* userdata2)
    {
        if (status != WGPURequestDeviceStatus_Success) {
            printf("Failed to get device: %.*s\n", (int)message.length, message.data);
            return;
        }

        device = dev;
        finish_device_setup();
    }


    static void onAdapterRequest(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void* userdata2)
    {
        if (status != WGPURequestAdapterStatus_Success) {
            printf("Failed to get adapter: %.*s\n", (int)message.length, message.data);
            return;
        }

        globalAdapter = adapter;

        WGPUDeviceDescriptor deviceDesc = {0};
        WGPURequestDeviceCallbackInfo deviceCallbackInfo = {0};
        deviceCallbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
        deviceCallbackInfo.callback = onDeviceRequest;
        wgpuAdapterRequestDevice(adapter, &deviceDesc, deviceCallbackInfo);
    }
#endif