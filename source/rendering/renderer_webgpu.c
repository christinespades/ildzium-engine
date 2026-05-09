#if defined(__EMSCRIPTEN__)
#include "pch.h"
#include "renderer_webgpu.h"
// in pch.h: #include <webgpu/webgpu.h>

extern CameraUBO cameraUBOData;

WGPUDevice device = NULL;
WGPUAdapter globalAdapter = NULL;
GPUState gpu_state = GPU_STATE_NOT_READY;
WGPUSurface surface = NULL;
WGPUTextureFormat swapchainFormat = WGPUTextureFormat_BGRA8Unorm;
WGPURenderPipeline pipeline = NULL;
WGPUBuffer vertexBuffer = NULL;
WGPUBindGroup bindGroup = NULL;

extern int g_width;  // in core/window.h
extern int g_height; // in core/window.h

static void onAdapterRequest(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void* userdata2);
static void onDeviceRequest(WGPURequestDeviceStatus status, WGPUDevice dev, WGPUStringView message, void* userdata1, void* userdata2);

void webgpu_init(void)
{
    printf("Initializing WebGPU (Emdawnwebgpu - C version)...\n");

    WGPUInstance instance = wgpuCreateInstance(NULL);
    if (!instance) {
        printf("Failed to create WebGPU instance\n");
        return;
    }

    WGPUEmscriptenSurfaceSourceCanvasHTMLSelector canvasSource = {0};
    canvasSource.chain.sType = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector;
    canvasSource.selector = (WGPUStringView){ .data = "#canvas", .length = WGPU_STRLEN };

    WGPUSurfaceDescriptor surfaceDesc = {0};
    surfaceDesc.nextInChain = &canvasSource.chain;
    surface = wgpuInstanceCreateSurface(instance, &surfaceDesc);
    if (!surface) {
        printf("Failed to create surface from canvas\n");
        return;
    }

    WGPURequestAdapterOptions adapterOpts = {0};
    adapterOpts.compatibleSurface = surface;

    WGPURequestAdapterCallbackInfo adapterCallbackInfo = {0};
    adapterCallbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
    adapterCallbackInfo.callback = onAdapterRequest;
    // You can pass userdata if needed (e.g., to signal completion)

    wgpuInstanceRequestAdapter(instance, &adapterOpts, adapterCallbackInfo);
    gpu_state = GPU_STATE_NOT_READY;

    // Simple spin-wait with yielding (works great in Emscripten main loop)
    while (gpu_state != GPU_STATE_READY) {
        wgpuInstanceProcessEvents(instance);   // This makes callbacks fire
        emscripten_sleep(10);                  // Yield to browser (~10ms)
    }

    printf("WebGPU initialization completed successfully\n");
}

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

    gpu_state = GPU_STATE_ADAPTER_OK;   // optional intermediate state
}

static void onDeviceRequest(WGPURequestDeviceStatus status, WGPUDevice dev, WGPUStringView message, void* userdata1, void* userdata2)
{
    if (status != WGPURequestDeviceStatus_Success) {
        printf("Failed to get device: %.*s\n", (int)message.length, message.data);
        gpu_state = GPU_STATE_NOT_READY;
        return;
    }

    device = dev;
    EM_ASM({
            console.log('✅ Device created from C (pointer:', $0, ')');

            // Try to hook into any global error we can
            if (typeof navigator !== 'undefined' && navigator.gpu) {
                console.log('WebGPU is available in browser');
            }
        }, device);
    queue = wgpuDeviceGetQueue(device);

    WGPUSurfaceCapabilities caps = {0};
    wgpuSurfaceGetCapabilities(surface, globalAdapter, &caps);
    swapchainFormat = caps.formats[0];

    WGPUSurfaceConfiguration config = {0};
    config.device       = device;
    config.format       = swapchainFormat;
    config.usage        = WGPUTextureUsage_RenderAttachment;
    config.width        = (uint32_t)g_width;      // must be > 0
    config.height       = (uint32_t)g_height;
    config.presentMode  = WGPUPresentMode_Fifo;
    config.alphaMode    = WGPUCompositeAlphaMode_Opaque;   // important for Emscripten

    // These two lines prevent many "assertion failed" crashes:
    config.viewFormatCount = 0;
    config.viewFormats     = NULL;

    wgpuSurfaceConfigure(surface, &config);
    printf("Surface successfully configured: %d x %d\n", g_width, g_height);

    /* Uniform buffer */
    WGPUBufferDescriptor bufDesc = {0};
    bufDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
    bufDesc.size = sizeof(CameraUBO);
    cameraBuffer = wgpuDeviceCreateBuffer(device, &bufDesc);

    /* Shader (using WGPUStringView) */
    const char* wgslCode =
        "struct Camera {\n"
        "    viewProj : mat4x4<f32>\n"
        "};\n"
        "@group(0) @binding(0) var<uniform> camera : Camera;\n"
        "struct VSOut {\n"
        "    @builtin(position) pos : vec4<f32>\n"
        "};\n"
        "@vertex fn vs_main(@location(0) position : vec3<f32>) -> VSOut {\n"
        "    var out : VSOut;\n"
        "    out.pos = camera.viewProj * vec4<f32>(position, 1.0);\n"
        "    return out;\n"
        "}\n"
        "@fragment fn fs_main() -> @location(0) vec4<f32> {\n"
        "    return vec4<f32>(0.8, 0.2, 0.2, 1.0);\n"
        "}\n";

    WGPUShaderSourceWGSL wgsl = {0};
    wgsl.chain.sType = WGPUSType_ShaderSourceWGSL;
    wgsl.code = (WGPUStringView){ .data = wgslCode, .length = WGPU_STRLEN };

    WGPUShaderModuleDescriptor shaderDesc = {0};
    shaderDesc.nextInChain = &wgsl.chain;
    shaderDesc.label = (WGPUStringView){ .data = "MainShader", .length = 10 };

    WGPUShaderModule shader = wgpuDeviceCreateShaderModule(device, &shaderDesc);

    printf("Shader module created (handle: %p)\n", (void*)shader);

    /* Bind group layout */
    WGPUBindGroupLayoutEntry entry = {0};
    entry.binding = 0;
    entry.visibility = WGPUShaderStage_Vertex;
    entry.buffer.type = WGPUBufferBindingType_Uniform;

    WGPUBindGroupLayoutDescriptor layoutDesc = {0};
    layoutDesc.entryCount = 1;
    layoutDesc.entries = &entry;

    WGPUBindGroupLayout bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &layoutDesc);

    WGPUPipelineLayoutDescriptor plDesc = {0};
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = &bindGroupLayout;
    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &plDesc);

    /* Vertex layout */
    WGPUVertexAttribute attrs[1] = {{
        .format = WGPUVertexFormat_Float32x3,
        .offset = 0,
        .shaderLocation = 0
    }};

    WGPUVertexBufferLayout vbLayout = {0};
    vbLayout.arrayStride = sizeof(float) * 3;
    vbLayout.attributeCount = 1;
    vbLayout.attributes = attrs;

    /* Pipeline */
    WGPUColorTargetState colorTarget = {0};
    colorTarget.format = swapchainFormat;

    WGPUFragmentState fragment = {0};
    fragment.module = shader;
    fragment.entryPoint = (WGPUStringView){ .data = "fs_main", .length = WGPU_STRLEN };
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;

    WGPUMultisampleState multisample = {0};
    multisample.count = 1;                    // 1 = no multisampling
    multisample.mask = 0xFFFFFFFF;
    multisample.alphaToCoverageEnabled = false;

    // Push error scope before pipeline creation
        EM_ASM({
            if (Module && Module['device']) {
                Module['device'].pushErrorScope('validation');
            }
        }, device);   // try with device pointer

    WGPURenderPipelineDescriptor pipeDesc = {0};
    pipeDesc.layout = pipelineLayout;
    pipeDesc.vertex.module = shader;
    pipeDesc.vertex.entryPoint = (WGPUStringView){ .data = "vs_main", .length = WGPU_STRLEN };
    pipeDesc.vertex.bufferCount = 1;
    pipeDesc.vertex.buffers = &vbLayout;
    pipeDesc.fragment = &fragment;
    pipeDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipeDesc.multisample = multisample;
    pipeDesc.label = (WGPUStringView){ .data = "RedTrianglePipeline", .length = WGPU_STRLEN };

    pipeline = wgpuDeviceCreateRenderPipeline(device, &pipeDesc);

    // Pop error scope after pipeline creation
    EM_ASM({
        if (Module && Module['device']) {
            Module['device'].popErrorScope().then((error) => {
                if (error) {
                    console.error('🚨 Pipeline Creation Error:', error.message);
                } else {
                    console.log('✅ Pipeline creation had no validation errors');
                }
            });
        }
    }, device);

    /* Vertex buffer */
    float vertices[] = {
        -0.8f, -0.8f, 0.0f,
         0.8f, -0.8f, 0.0f,
         0.0f,  0.8f, 0.0f
    };

    WGPUBufferDescriptor vbDesc = {0};
    vbDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    vbDesc.size = sizeof(vertices);
    vertexBuffer = wgpuDeviceCreateBuffer(device, &vbDesc);
    wgpuQueueWriteBuffer(queue, vertexBuffer, 0, vertices, sizeof(vertices));

    /* Bind group */
    WGPUBindGroupEntry bgEntry = {0};
    bgEntry.binding = 0;
    bgEntry.buffer = cameraBuffer;
    bgEntry.size = sizeof(CameraUBO);

    WGPUBindGroupDescriptor bgDesc = {0};
    bgDesc.layout = bindGroupLayout;
    bgDesc.entryCount = 1;
    bgDesc.entries = &bgEntry;

    bindGroup = wgpuDeviceCreateBindGroup(device, &bgDesc);

    create_sky_pipeline_webgpu();
    init_lights_webgpu(device);
    init_model_system();
    ui_renderer_init();

    gpu_state = GPU_STATE_READY;
    printf("WebGPU fully ready\n");

}
#endif