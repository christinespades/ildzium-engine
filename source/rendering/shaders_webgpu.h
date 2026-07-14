#pragma once
#include "scene/model_webgpu.h"
#include "scene/sky_webgpu.h"
#include "ui/ui.h"

static void onShaderCompilation(WGPUCompilationInfoRequestStatus status, 
                                WGPUCompilationInfo const* info, 
                                void* userdata1, 
                                void* userdata2);
WGPUShaderModule create_shader_module(WGPUDevice device, const char* wgslCode, const char* label);