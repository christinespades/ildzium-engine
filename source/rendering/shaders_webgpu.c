#include "pch.h"

#ifdef __EMSCRIPTEN__
    #include "rendering/shaders_webgpu.h"

    static void onShaderCompilation(WGPUCompilationInfoRequestStatus status, 
                                    WGPUCompilationInfo const* info, 
                                    void* userdata1, 
                                    void* userdata2) {
        const char* shaderLabel = (const char*)userdata1;
        
        if (status != WGPUCompilationInfoRequestStatus_Success) return;

        for (uint32_t i = 0; i < info->messageCount; ++i) {
            WGPUCompilationMessage const* m = &info->messages[i];
            
            // Print it clearly to the console
            printf("[WGSL %s] %s at line %llu, col %llu: %.*s\n",
                   shaderLabel,
                   m->type == WGPUCompilationMessageType_Error ? "ERROR" : "WARNING",
                   m->lineNum, m->linePos,
                   (int)m->message.length, m->message.data);
        }
    }

    WGPUShaderModule create_shader_module(WGPUDevice device, const char* wgslCode, const char* label) {
        WGPUShaderSourceWGSL wgslSource = {
            .chain = { .next = NULL, .sType = WGPUSType_ShaderSourceWGSL },
            .code = (WGPUStringView){ .data = wgslCode, .length = WGPU_STRLEN }
        };

        WGPUShaderModuleDescriptor desc = {
            .nextInChain = (WGPUChainedStruct*)&wgslSource,
            .label = (WGPUStringView){ .data = label, .length = WGPU_STRLEN }
        };

        WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &desc);

        WGPUCompilationInfoCallbackInfo callbackInfo = {
            .nextInChain = NULL,
            .mode = WGPUCallbackMode_AllowProcessEvents,
            .callback = onShaderCompilation,
            .userdata1 = (void*)label
        };

        wgpuShaderModuleGetCompilationInfo(module, callbackInfo);

        return module;
    }
}
#endif