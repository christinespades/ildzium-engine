#include "pch.h"
#ifdef __EMSCRIPTEN__
    #include "ui/ui_renderer_webgpu.h"
    uint32_t* ui_framebuffer = NULL;
    #include "rendering/renderer_webgpu.h"
    extern WGPUDevice device;
    extern WGPUQueue queue;
    extern WGPUTextureFormat swapchainFormat;

    WGPURenderPipeline uiPipeline      = NULL;
    WGPUBindGroup      uiBindGroup     = NULL;
    WGPUBuffer         uiVertexBuffer  = NULL;
    WGPUTexture        uiTexture       = NULL;
    WGPUTextureView    uiTextureView   = NULL;
    WGPUSampler        uiSampler       = NULL;

    static uint32_t currentUiWidth = 1280;
    static uint32_t currentUiHeight = 720;

    // when size changes
    static void create_ui_texture(uint32_t width, uint32_t height)
    {
        if (uiTexture) wgpuTextureRelease(uiTexture);
        if (uiTextureView) wgpuTextureViewRelease(uiTextureView);

        WGPUTextureDescriptor texDesc = {0};
        texDesc.usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding;
        texDesc.dimension = WGPUTextureDimension_2D;
        texDesc.size = (WGPUExtent3D){width, height, 1};
        texDesc.format = WGPUTextureFormat_RGBA8Unorm;
        texDesc.mipLevelCount = 1;
        texDesc.sampleCount = 1;

        uiTexture = wgpuDeviceCreateTexture(device, &texDesc);

        WGPUTextureViewDescriptor viewDesc = {0};
        viewDesc.format = WGPUTextureFormat_RGBA8Unorm;
        viewDesc.dimension = WGPUTextureViewDimension_2D;
        viewDesc.mipLevelCount = 1;
        viewDesc.arrayLayerCount = 1;
        viewDesc.aspect = WGPUTextureAspect_All;

        uiTextureView = wgpuTextureCreateView(uiTexture, &viewDesc);

        currentUiWidth = width;
        currentUiHeight = height;
    }

    typedef struct { float x, y, u, v; } Vertex;
    Vertex quadVerts[4] = {
        {-1.f, -1.f, 0.f, 1.f},   // bottom-left in NDC
        { 1.f, -1.f, 1.f, 1.f},
        {-1.f,  1.f, 0.f, 0.f},   // top-left in NDC
        { 1.f,  1.f, 1.f, 0.f}
    };

    void ui_renderer_upload(uint32_t* ui_pixels, uint32_t width, uint32_t height) {
        if (!ui_pixels) {
            printf("No UI pixels\n");
            return;
        }

        // Recreate texture if size changed
        if (width != currentUiWidth || height != currentUiHeight) {
            create_ui_texture(width, height);
            // Recreate bind group with new texture view
            // (for simplicity we recreate bind group here - you can optimize later)
            if (uiBindGroup) wgpuBindGroupRelease(uiBindGroup);
            // ... recreate bind group same as in init (copy the code or make a helper)
            // For now, call ui_renderer_init() again is acceptable for UI
            ui_renderer_init();   // simple but works
        }
        WGPUTexelCopyTextureInfo dst = {
            .texture = uiTexture,
            .mipLevel = 0,
            .origin = {0, 0, 0},
            .aspect = WGPUTextureAspect_All
        };

        WGPUTexelCopyBufferLayout layout = {
            .offset = 0,
            .bytesPerRow = width * 4,
            .rowsPerImage = height
        };

        WGPUExtent3D size = { 
            .width = width, 
            .height = height, 
            .depthOrArrayLayers = 1 
        };

        wgpuQueueWriteTexture(queue, &dst, ui_pixels, (size_t)width * height * 4, &layout, &size);
    }

    void ui_renderer_init() {
    printf("Creating WebGPU UI Renderer...\n");

    // Initial texture (default size)
    create_ui_texture(1280, 720);

    // Sampler
    WGPUSamplerDescriptor sampDesc = {0};
    sampDesc.magFilter = WGPUFilterMode_Nearest;
    sampDesc.minFilter = WGPUFilterMode_Nearest;
    sampDesc.addressModeU = WGPUAddressMode_ClampToEdge;
    sampDesc.addressModeV = WGPUAddressMode_ClampToEdge;
    sampDesc.addressModeW = WGPUAddressMode_ClampToEdge;
    sampDesc.maxAnisotropy = 1;
    uiSampler = wgpuDeviceCreateSampler(device, &sampDesc);

    // Vertex buffer (full-screen quad)
    float quadVerts[] = {
        -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 0.0f
    };

    WGPUBufferDescriptor vbDesc = {0};
    vbDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    vbDesc.size = sizeof(quadVerts);
    uiVertexBuffer = wgpuDeviceCreateBuffer(device, &vbDesc);
    wgpuQueueWriteBuffer(queue, uiVertexBuffer, 0, quadVerts, sizeof(quadVerts));

    // === Bind Group Layout ===
    WGPUBindGroupLayoutEntry entries[2] = {0};

    // Texture Binding
    entries[0].binding = 0;
    entries[0].visibility = WGPUShaderStage_Fragment;
    entries[0].texture.sampleType = WGPUTextureSampleType_Float;
    entries[0].texture.viewDimension = WGPUTextureViewDimension_2D;

    // Sampler Binding (MISSING IN YOUR ORIGINAL CODE)
    entries[1].binding = 1;
    entries[1].visibility = WGPUShaderStage_Fragment;
    entries[1].sampler.type = WGPUSamplerBindingType_Filtering;

    WGPUBindGroupLayoutDescriptor layoutDesc = {0};
    layoutDesc.entryCount = 2; // Change to 2
    layoutDesc.entries = entries;

    WGPUBindGroupLayout bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &layoutDesc);

    // === Pipeline Layout ===
    WGPUPipelineLayoutDescriptor plDesc = {0};
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = &bindGroupLayout;
    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &plDesc);

    // === Shader ===
    const char* wgsl = 
        "@group(0) @binding(0) var uiTexture : texture_2d<f32>;\n"
        "@group(0) @binding(1) var uiSampler : sampler;\n"

        "struct VSOut {\n"
        "    @builtin(position) pos : vec4<f32>,\n"
        "    @location(0) uv : vec2<f32>\n"
        "};\n"

        "@vertex fn vs_main(@location(0) pos : vec2<f32>, @location(1) uv : vec2<f32>) -> VSOut {\n"
        "    var out : VSOut;\n"
        "    out.pos = vec4<f32>(pos, 0.0, 1.0);\n"
        "    out.uv = -uv;\n"
        "    return out;\n"
        "}\n"

        "@fragment fn fs_main(@location(0) uv : vec2<f32>) -> @location(0) vec4<f32> {\n"
        "    return vec4<f32>(1.0, 1.0, 1.0, 1.0);\n"
        "}\n";

    WGPUShaderModule shader = create_shader_module(device, wgsl, "UIShader");

    // Vertex layout (pos + uv)
    WGPUVertexAttribute attrs[2] = {
        { .format = WGPUVertexFormat_Float32x2, .offset = 0,  .shaderLocation = 0 },
        { .format = WGPUVertexFormat_Float32x2, .offset = 8,  .shaderLocation = 1 }
    };
    WGPUVertexBufferLayout vbLayout = {
        .arrayStride = 16,
        .attributeCount = 2,
        .attributes = attrs
    };

    WGPUColorTargetState colorTarget = {0};
    colorTarget.format = swapchainFormat;

    // 1. Define the blend state as a separate variable
    WGPUBlendState blendState = {
        .color = { 
            .operation = WGPUBlendOperation_Add, 
            .srcFactor = WGPUBlendFactor_SrcAlpha, 
            .dstFactor = WGPUBlendFactor_OneMinusSrcAlpha 
        },
        .alpha = { 
            .operation = WGPUBlendOperation_Add, 
            .srcFactor = WGPUBlendFactor_One, 
            .dstFactor = WGPUBlendFactor_Zero 
        }
    };

    // 2. Assign the address of that variable to the target
    colorTarget.blend = &blendState;

    const char* vs_entry = "vs_main";
    const char* fs_entry = "fs_main";

    WGPUFragmentState fragment = {
        .module = shader,
        .entryPoint = (WGPUStringView){.data = fs_entry, .length = strlen(fs_entry)},
        .targetCount = 1,
        .targets = &colorTarget
    };

    WGPUDepthStencilState depthStencil = {0};
    depthStencil.format = WGPUTextureFormat_Depth24Plus;
    depthStencil.depthWriteEnabled = true;
    depthStencil.depthCompare = WGPUCompareFunction_Less;

    WGPURenderPipelineDescriptor desc = {0};
    desc.layout = pipelineLayout;

    desc.vertex.module = shader;
    desc.vertex.entryPoint = (WGPUStringView){.data = vs_entry, .length = strlen(vs_entry)};
    desc.vertex.bufferCount = 1;
    desc.vertex.buffers = &vbLayout;
    desc.fragment = &fragment;
    desc.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
    //desc.primitive.cullMode = WGPUCullMode_None;
    desc.multisample.count = 1;
    desc.depthStencil = &depthStencil;
    desc.label = (WGPUStringView){ .data = "UIPipeline", .length = 10 };

    uiPipeline = wgpuDeviceCreateRenderPipeline(device, &desc);

    // === Create Bind Group ===
    WGPUBindGroupEntry bgEntries[2] = {0};

    // Texture View
    bgEntries[0].binding = 0;
    bgEntries[0].textureView = uiTextureView;

    // Sampler (MISSING IN YOUR ORIGINAL CODE)
    bgEntries[1].binding = 1;
    bgEntries[1].sampler = uiSampler;

    WGPUBindGroupDescriptor bgDesc = {0};
    bgDesc.layout = bindGroupLayout;
    bgDesc.entryCount = 2; // Change to 2
    bgDesc.entries = bgEntries;

    uiBindGroup = wgpuDeviceCreateBindGroup(device, &bgDesc);

    printf("WebGPU UI pipeline + bind group created with alpha blending\n");
    }

    void ui_renderer_draw(WGPURenderPassEncoder pass)
    {
        if (!uiPipeline || !uiBindGroup) return;
        wgpuRenderPassEncoderSetPipeline(pass, uiPipeline);
        wgpuRenderPassEncoderSetBindGroup(pass, 0, uiBindGroup, 0, NULL);
        wgpuRenderPassEncoderSetVertexBuffer(pass, 0, uiVertexBuffer, 0, sizeof(quadVerts));
        wgpuRenderPassEncoderDraw(pass, 4, 1, 0, 0);
    }

    void ui_renderer_cleanup(void)
    {
        if (uiPipeline) wgpuRenderPipelineRelease(uiPipeline);
        if (uiBindGroup) wgpuBindGroupRelease(uiBindGroup);
        if (uiVertexBuffer) wgpuBufferRelease(uiVertexBuffer);
        if (uiTextureView) wgpuTextureViewRelease(uiTextureView);
        if (uiTexture) wgpuTextureRelease(uiTexture);
        if (uiSampler) wgpuSamplerRelease(uiSampler);
    }
#endif