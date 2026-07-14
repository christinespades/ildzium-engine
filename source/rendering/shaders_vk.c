#include "pch.h"

#ifndef __EMSCRIPTEN__
    #include "rendering/shaders_vk.h"

    uint32_t* load_spirv(const char* filename, size_t* out_size)
    {
        FILE* file = fopen(filename, "rb");
        if (!file) {
            LOGE("Failed to open shader: %s", filename);
            *out_size = 0;
            return NULL;
        }

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (file_size % 4 != 0) {
            LOGE("Invalid SPIR-V file size: %s", filename);
            fclose(file);
            *out_size = 0;
            return NULL;
        }

        uint32_t* code = malloc(file_size);
        fread(code, 1, file_size, file);
        fclose(file);

        *out_size = file_size;
        return code;
    }

    typedef void (*pipeline_fn)(void);

    typedef struct {
        const char* prefix;
        size_t      prefix_len;
        pipeline_fn fn;
    } ShaderDispatch;

    extern void create_debug_pipelines(void);
    extern void create_model_pipeline(void);
    extern void create_ui_pipeline(void);
    extern void create_sky_pipeline(void);

    static const ShaderDispatch dispatch_table[] = {
        { "debug", sizeof("debug") - 1, create_debug_pipelines },
        { "debug_geometry", sizeof("debug_geometry") - 1, create_debug_pipelines },
        { "model", sizeof("model") - 1, create_model_pipeline },
        { "ui", sizeof("ui") - 1, create_ui_pipeline    },
        { "sky", sizeof("sky") - 1, create_sky_pipeline   },
    };

    static const size_t dispatch_count =
        sizeof(dispatch_table) / sizeof(dispatch_table[0]);

    static const char* get_filename(const char* path)
    {
        const char* slash1 = strrchr(path, '/');
        const char* slash2 = strrchr(path, '\\');
        const char* slash = slash1 > slash2 ? slash1 : slash2;
        return slash ? slash + 1 : path;
    }

    void on_shader_changed(const char* path)
    {
        const char* filename = get_filename(path);
        LOGI("Hot-reloading shader: %s", filename);
        for (size_t i = 0; i < dispatch_count; ++i)
        {
            const ShaderDispatch* entry = &dispatch_table[i];
            if (strncmp(filename, entry->prefix, entry->prefix_len) == 0)
            {
                entry->fn();
                return;
            }
        }
        LOGE("No pipeline mapped for: %s", filename);
    }

    void load_shader_pair(const char* shaderName, VkShaderModule* outVertModule, VkShaderModule* outFragModule)
    {
        size_t vert_size, frag_size;
        char vert_path[256];
        char frag_path[256];

        snprintf(vert_path, sizeof(vert_path), "../../shaders/%s.vert.spv", shaderName);
        snprintf(frag_path, sizeof(frag_path), "../../shaders/%s.frag.spv", shaderName);

        uint32_t* vert_code = load_spirv(vert_path, &vert_size);
        uint32_t* frag_code = load_spirv(frag_path, &frag_size);

        if (!vert_code || !frag_code) {
            LOGE("Failed to load %s shaders!", shaderName);
            exit(1);
        }

        VkShaderModuleCreateInfo sm = {0};
        sm.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        sm.codeSize = vert_size; 
        sm.pCode = vert_code;
        if (vkCreateShaderModule(vk_device, &sm, NULL, outVertModule) != VK_SUCCESS) {
            LOGE("Failed to create vertex shader module for %s", shaderName);
            exit(1);
        }
        free(vert_code);

        // Create Fragment Shader Module
        sm.codeSize = frag_size; 
        sm.pCode = frag_code;
        if (vkCreateShaderModule(vk_device, &sm, NULL, outFragModule) != VK_SUCCESS) {
            LOGE("Failed to create fragment shader module for %s", shaderName);
            exit(1);
        }
        free(frag_code);
    }

    void load_compute_shader(const char* shaderName, VkShaderModule* outComputeModule)
    {
        size_t comp_size;
        char comp_path[256];

        snprintf(comp_path, sizeof(comp_path), "../../shaders/%s.comp.spv", shaderName);

        uint32_t* comp_code = load_spirv(comp_path, &comp_size);

        if (!comp_code) {
            LOGE("Failed to load compute shader %s!", shaderName);
            exit(1);
        }

        VkShaderModuleCreateInfo sm = {0};
        sm.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        sm.codeSize = comp_size;
        sm.pCode = comp_code;

        if (vkCreateShaderModule(vk_device, &sm, NULL, outComputeModule) != VK_SUCCESS) {
            LOGE("Failed to create compute shader module for %s", shaderName);
            free(comp_code);
            exit(1);
        }

        free(comp_code);
    }
#endif