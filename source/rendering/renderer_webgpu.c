#if defined(__EMSCRIPTEN__)
#include "pch.h"
#include "rendering/device.h"
#include "scene/camera.h"
#include "renderer_webgpu.h"
// in pch.h: #include <webgpu/webgpu.h>

extern CameraUBO cameraUBOData;

WGPUDevice device = NULL;
WGPUSurface surface = NULL;
WGPUTextureFormat swapchainFormat = WGPUTextureFormat_BGRA8Unorm;
WGPURenderPipeline pipeline = NULL;
WGPUBuffer vertexBuffer = NULL;
WGPUBindGroup bindGroup = NULL;

static int width = 1280;
static int height = 720;

/* Callback functions */
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

    /* === Surface from Canvas (Emscripten-specific) === */
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

    /* === Request Adapter (modern callback style) === */
    WGPURequestAdapterOptions adapterOpts = {0};
    adapterOpts.compatibleSurface = surface;

    WGPURequestAdapterCallbackInfo adapterCallbackInfo = {0};
    adapterCallbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;   /* or WGPUCallbackMode_WaitAnyOnly if you poll */
    adapterCallbackInfo.callback = onAdapterRequest;
    adapterCallbackInfo.userdata1 = NULL;

    wgpuInstanceRequestAdapter(instance, &adapterOpts, adapterCallbackInfo);
    /* Note: This is asynchronous. The real init continues in the callback. */
}

static void onAdapterRequest(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void* userdata2)
{
    (void)userdata1; (void)userdata2;

    if (status != WGPURequestAdapterStatus_Success) {
        printf("Failed to get adapter: %.*s\n", (int)message.length, message.data);
        return;
    }

    WGPUDeviceDescriptor deviceDesc = {0};

    WGPURequestDeviceCallbackInfo deviceCallbackInfo = {0};
    deviceCallbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
    deviceCallbackInfo.callback = onDeviceRequest;
    deviceCallbackInfo.userdata1 = NULL;

    wgpuAdapterRequestDevice(adapter, &deviceDesc, deviceCallbackInfo);
}

static void onDeviceRequest(WGPURequestDeviceStatus status, WGPUDevice dev, WGPUStringView message, void* userdata1, void* userdata2)
{
    (void)userdata1; (void)userdata2;

    if (status != WGPURequestDeviceStatus_Success) {
        printf("Failed to get device: %.*s\n", (int)message.length, message.data);
        return;
    }

    device = dev;
    queue = wgpuDeviceGetQueue(device);

    /* Configure surface */
    WGPUSurfaceConfiguration config = {0};
    config.device = device;
    config.format = swapchainFormat;
    config.usage = WGPUTextureUsage_RenderAttachment;
    config.width = (uint32_t)width;
    config.height = (uint32_t)height;
    config.presentMode = WGPUPresentMode_Fifo;

    wgpuSurfaceConfigure(surface, &config);

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

    WGPUShaderModule shader = wgpuDeviceCreateShaderModule(device, &shaderDesc);

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

    WGPURenderPipelineDescriptor pipeDesc = {0};
    pipeDesc.layout = pipelineLayout;
    pipeDesc.vertex.module = shader;
    pipeDesc.vertex.entryPoint = (WGPUStringView){ .data = "vs_main", .length = WGPU_STRLEN };
    pipeDesc.vertex.bufferCount = 1;
    pipeDesc.vertex.buffers = &vbLayout;
    pipeDesc.fragment = &fragment;
    pipeDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;

    pipeline = wgpuDeviceCreateRenderPipeline(device, &pipeDesc);

    /* Vertex buffer */
    float vertices[] = { 0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f };

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

    printf("WebGPU initialized successfully\n");
}
#endif