#pragma once
#include "core/memory.h"

extern uint32_t* ui_framebuffer;

#ifdef __EMSCRIPTEN__
    extern WGPURenderPipeline uiPipeline;
    extern WGPUBindGroup     uiBindGroup;
    extern WGPUBuffer        uiVertexBuffer;
    extern WGPUTexture       uiTexture;
    extern WGPUTextureView   uiTextureView;
    extern WGPUSampler       uiSampler;
#else
	#include "rendering/renderer.h"
	#include "rendering/shaders.h"
#endif

void ui_renderer_init(void);
void ui_renderer_upload(uint32_t* ui_pixels, uint32_t width, uint32_t height);
#ifdef __EMSCRIPTEN__
	void ui_renderer_draw(WGPURenderPassEncoder pass);
	void ui_renderer_cleanup(void);
#else
	void ui_renderer_draw(VkCommandBuffer cmd);
	void ui_renderer_resize();
#endif