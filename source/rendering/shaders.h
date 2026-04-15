#pragma once
#include "scene/model.h"
#include "scene/sky.h"
#include "ui/ui.h"

#ifdef __EMSCRIPTEN__
	static void onShaderCompilation(WGPUCompilationInfoRequestStatus status, 
	                                WGPUCompilationInfo const* info, 
	                                void* userdata1, 
	                                void* userdata2);
	WGPUShaderModule create_shader_module(WGPUDevice device, const char* wgslCode, const char* label);
#else
	uint32_t* load_spirv(const char* filename, size_t* out_size);
	void on_shader_changed(const char* path);
#endif