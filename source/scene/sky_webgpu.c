#include "pch.h"
#if defined(__EMSCRIPTEN__)
    #include "scene/sky_webgpu.h"

    extern WGPUDevice device;
    extern WGPUQueue queue;
    extern WGPUTextureFormat swapchainFormat;

    WGPURenderPipeline skyPipeline = NULL;
    WGPUBindGroup     skyBindGroup = NULL;
    WGPUBuffer        skyVertexBuffer = NULL;
    WGPUBuffer        skyUBOBuffer = NULL;   // we'll create this later

    extern SkyUBO skyUBOData; 

    void update_sky_ubo_webgpu(void) {
        if (!skyUBOBuffer) return;

        // WebGPU requires buffers to be updated via the Queue
        wgpuQueueWriteBuffer(
            queue, 
            skyUBOBuffer, 
            0, 
            &skyUBOData, 
            sizeof(SkyUBO)
        );
    }

    static void create_sky_geometry(void)
    {
        // Fullscreen triangle in clip space (covers entire screen reliably)
        float verts[] = {
            -1.0f, -1.0f,     // bottom-left
             3.0f, -1.0f,     // bottom-right (extended)
            -1.0f,  3.0f      // top-left (extended)
        };

        WGPUBufferDescriptor desc = {0};
        desc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        desc.size = sizeof(verts);
        skyVertexBuffer = wgpuDeviceCreateBuffer(device, &desc);
        wgpuQueueWriteBuffer(queue, skyVertexBuffer, 0, verts, sizeof(verts));
    }

    void create_sky_bind_group(void) {
        WGPUBufferDescriptor bufDesc = {0};
        bufDesc.label = (WGPUStringView){ .data = "SkyUBOBuffer", .length = 12 };
        bufDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
        bufDesc.size = 256;
        
        skyUBOBuffer = wgpuDeviceCreateBuffer(device, &bufDesc);
        WGPUBindGroupLayout skyLayout = wgpuRenderPipelineGetBindGroupLayout(skyPipeline, 0);

        WGPUBindGroupEntry entry = {
            .binding = 0,
            .buffer = skyUBOBuffer,
            .offset = 0,
            .size = sizeof(SkyUBO)
        };

        WGPUBindGroupDescriptor bgDesc = {
            .label = (WGPUStringView){ .data = "SkyBindGroup", .length = 12 },
            .layout = skyLayout, // <--- Use the layout we just got
            .entryCount = 1,
            .entries = &entry
        };

        if (skyUBOBuffer == NULL) printf("ERROR: skyUBOBuffer is NULL\n");
        if (skyLayout == NULL) printf("ERROR: skyLayout is NULL\n");

        skyBindGroup = wgpuDeviceCreateBindGroup(device, &bgDesc);

        // After creating the bind group, you should release the layout handle 
        // (the bind group keeps its own internal reference)
        wgpuBindGroupLayoutRelease(skyLayout);
    }

    void create_sky_pipeline_webgpu(void)
    {
        printf("Creating WebGPU Sky Pipeline...\n");

        const char* wgslCode =
            "struct SkyUBO {\n"
            "    time : f32,\n"
            "    timeOfDay : f32,\n"
            "    dayNightBlend : f32,\n"
            "    nebulaSpeed : f32,\n"
            "    nebulaScale : f32,\n"
            "    nebulaIntensity : f32,\n"
            "    nebulaLayerCount : f32,\n"
            "    starCount : f32,\n"
            "    starBrightness : f32,\n"
            "    starTwinkleSpeed : f32,\n"
            "    starSize : f32,\n"
            "    auroraIntensity : f32,\n"
            "    auroraSpeed : f32,\n"
            "    vignetteStrength : f32,\n"
            "    overallBrightness : f32,\n"
            "    padding : f32,\n" // Added padding to align the following vec4s
            "    nebulaColorNight : vec4<f32>,\n"
            "    nebulaColorDay : vec4<f32>,\n"
            "    auroraColor : vec4<f32>,\n"
            "    inverseView : mat4x4<f32>,\n"
            "};\n"
            "\n"
            "@group(0) @binding(0) var<uniform> ubo : SkyUBO;\n"
            "\n"
            "struct VSOut {\n"
            "    @builtin(position) pos : vec4<f32>,\n"
            "    @location(0) uv : vec2<f32>,\n" // Added comma for safety
            "};\n"
            "\n"
            "@vertex fn vs_main(@location(0) in_pos : vec2<f32>) -> VSOut {\n"
            "    var out : VSOut;\n"
            "    out.pos = vec4<f32>(in_pos, 0.0, 1.0);\n"
            "    out.uv = (in_pos + 1.0) * 0.5;\n"
            "    return out;\n"
            "}\n"
            "\n"
            "@fragment fn fs_main(@location(0) uv : vec2<f32>) -> @location(0) vec4<f32> {\n"
            "    let rd = normalize(vec3<f32>(uv * 2.0 - 1.0, 1.6));\n"
            "    let dir = (ubo.inverseView * vec4<f32>(rd, 0.0)).xyz;\n"
            "    var col = vec3<f32>(0.05, 0.08, 0.85);\n"
            "    let nebula = mix(ubo.nebulaColorNight.rgb, ubo.nebulaColorDay.rgb, ubo.dayNightBlend);\n"
            "    col += nebula * ubo.nebulaIntensity;\n"
            "    return vec4<f32>(col * ubo.overallBrightness, 1.0);\n"
            "}\n";

        WGPUShaderModule shader = create_shader_module(device, wgslCode, "SkyShader");
        if (!shader) {
            printf("ERROR: Failed to create sky shader module!\n");
            return;
}

       // 1. CREATE BIND GROUP LAYOUT (Required for UBO)
        WGPUBindGroupLayoutEntry bglEntry = {0};
        bglEntry.binding = 0;
        bglEntry.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
        bglEntry.buffer.type = WGPUBufferBindingType_Uniform;

        WGPUBindGroupLayoutDescriptor bglDesc = {0};
        bglDesc.entryCount = 1;
        bglDesc.entries = &bglEntry;
        WGPUBindGroupLayout skyBGL = wgpuDeviceCreateBindGroupLayout(device, &bglDesc);

        // 2. CREATE PIPELINE LAYOUT
        WGPUPipelineLayoutDescriptor plDesc = {0};
        plDesc.bindGroupLayoutCount = 1;
        plDesc.bindGroupLayouts = &skyBGL;
        WGPUPipelineLayout skyLayout = wgpuDeviceCreatePipelineLayout(device, &plDesc);

        // Vertex attributes
        WGPUVertexAttribute attr = {
            .format = WGPUVertexFormat_Float32x2,
            .offset = 0,
            .shaderLocation = 0
        };
        WGPUVertexBufferLayout vbLayout = {
            .arrayStride = 8,
            .attributeCount = 1,
            .attributes = &attr
        };

        WGPUColorTargetState colorTarget = { .format = swapchainFormat };

        WGPUFragmentState fragment = {
            .module = shader,
            .entryPoint = (WGPUStringView){.data = "fs_main", .length = 7},
            .targetCount = 1,
            .targets = &colorTarget
        };

        WGPURenderPipelineDescriptor desc = {0};
        desc.layout = skyLayout; // <--- ASSIGN THE LAYOUT
        desc.vertex.module = shader;
        desc.vertex.entryPoint = (WGPUStringView){.data = "vs_main", .length = 7};
        desc.vertex.bufferCount = 1;
        desc.vertex.buffers = &vbLayout;
        desc.fragment = &fragment;
        desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
        desc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
        desc.primitive.frontFace = WGPUFrontFace_CCW;           // or CW – try both
        desc.primitive.cullMode = WGPUCullMode_None;

        // Depth-stencil (set to NULL or fill it if you have depth)
        desc.depthStencil = NULL;   // or &depthStencilState
        desc.multisample.count                  = 1;
        desc.multisample.mask                   = 0xFFFFFFFF;
        desc.multisample.alphaToCoverageEnabled = false;
        desc.label = (WGPUStringView){ .data = "SkyPipeline", .length = 11 };

        skyPipeline = wgpuDeviceCreateRenderPipeline(device, &desc);
        
        // Cleanup temporary layout objects
        wgpuBindGroupLayoutRelease(skyBGL);
        wgpuPipelineLayoutRelease(skyLayout);

        create_sky_geometry();
        create_sky_bind_group();
        printf("WebGPU Sky pipeline created successfully\n");
    }

    void sky_draw_webgpu(WGPURenderPassEncoder pass)
    {
        if (!skyPipeline || !skyBindGroup) {
            printf ("no sky!");
            return;
        }

        wgpuRenderPassEncoderSetPipeline(pass, skyPipeline);
        wgpuRenderPassEncoderSetBindGroup(pass, 0, skyBindGroup, 0, NULL);
        wgpuRenderPassEncoderSetVertexBuffer(pass, 0, skyVertexBuffer, 0, sizeof(float)*6);  // 3 verts * 2 floats
        wgpuRenderPassEncoderDraw(pass, 3, 1, 0, 0);   // 3 vertices, not 4
    }

#endif