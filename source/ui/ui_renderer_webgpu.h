#pragma once
#include "core/memory.h"

extern uint32_t* ui_framebuffer;

extern WGPURenderPipeline uiPipeline;
extern WGPUBindGroup     uiBindGroup;
extern WGPUBuffer        uiVertexBuffer;
extern WGPUTexture       uiTexture;
extern WGPUTextureView   uiTextureView;
extern WGPUSampler       uiSampler;

void ui_renderer_init(void);
void ui_renderer_upload(uint32_t* ui_pixels, uint32_t width, uint32_t height);
void ui_renderer_draw(WGPURenderPassEncoder pass);
void ui_renderer_cleanup(void);