#pragma once
#include "core/math.h"
#include "rendering/shaders.h"
#include "scene/lights.h"
#include "scene/model_instances.h"
#include "scene/model_wgsl.h"
#ifndef __EMSCRIPTEN__
    #include "rendering/renderer_vulkan.h"
#endif
    typedef struct {
        float x, y, z;
        float nx, ny, nz;
        float u, v;
    } Vertex3D;

    typedef struct {
    #ifdef __EMSCRIPTEN__
        WGPUBuffer vertexBuffer;
        WGPUBuffer indexBuffer;
    #else
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexMemory;
    #endif
        uint32_t vertexCount;
        uint32_t indexCount;
    } Mesh;

    typedef struct {
        float model[16];     // mat4 - 64 bytes
        float color[4];      // vec4 - 16 bytes
    } InstanceData; // Perfectly matches old 80-byte shader alignment

    typedef struct {
        float model[16];
        float color[4];
        
        // CPU-only bookkeeping data (Never sent to the GPU)
        uint32_t model_index;   
        float bounding_center[3]; 
        float bounding_radius;  
    } CPUInstanceData;

    // One unique loaded model (can have 1 mesh for now)
    typedef struct {
        char* name;
        Mesh  mesh;
        uint32_t instanceOffset;
        uint32_t instanceCount;
        bool isLoaded;
        
        // Cache the model geometry's intrinsic bounding bounds
        float local_center[3];
        float local_radius;
    } Model;

    typedef struct {
        Model* models;                 // dynamic array
        uint32_t modelCount;
        uint32_t modelCapacity;

        CPUInstanceData* instances;       // still one big buffer for the GPU
        uint32_t instanceCapacity;
        uint32_t instanceCount;        // total live instances across ALL models
    #ifdef __EMSCRIPTEN__
        WGPUBuffer instanceBuffer;
    #else
        VkBuffer instanceBuffer;
        VkDeviceMemory instanceMemory;
        VkDeviceSize instanceBufferSize;
    #endif
    } ModelSystem;

    extern ModelSystem g_model_system;

#ifndef __EMSCRIPTEN__
    VkPipeline modelPipeline;
    VkPipelineLayout modelPipelineLayout;
    VkDescriptorSet modelDescriptorSet;
    VkDescriptorSetLayout modelDescriptorSetLayout;
    VkDescriptorPool modelDescriptorPool;

    void create_model_descriptors(void);
    void update_model_descriptor(void);
#endif
    void init_model_system(void);
    void load_model(const char* glb_path);
    Model* find_or_load_model(const char* name);     // returns Model*
    void destroy_model(Model* model);
    void cleanup_model_system(void);