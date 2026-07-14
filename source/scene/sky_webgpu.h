#pragma once

#include "rendering/renderer_webgpu.h"
#include "scene/camera.h"
#include "scene/sky.h"

extern WGPURenderPipeline skyPipeline;
extern WGPUBindGroup     skyBindGroup;
extern WGPUBuffer        skyVertexBuffer;

void create_sky_pipeline_webgpu(void);
void create_sky_bind_group(void);
void sky_draw_webgpu(WGPURenderPassEncoder pass);
void update_sky_ubo_webgpu(void);