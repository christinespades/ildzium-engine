#pragma once
#include <vulkan/vulkan.h>

typedef struct {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
} Vertex3D;

typedef struct {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexMemory;
    uint32_t vertexCount;
    uint32_t indexCount;
} Mesh;

typedef struct {
    float model[16];     // mat4 - 64 bytes
    float color[4];      // vec4 - 16 bytes
} InstanceData;

// One unique loaded model (can have 1 mesh for now)
typedef struct {
    char* name;                    // e.g. "cs_goddess_statue"
    Mesh  mesh;                    // ← changed: single Mesh for now (easier)
    uint32_t instanceOffset;       // start index in the big instance buffer
    uint32_t instanceCount;        // how many instances of THIS model
} Model;

typedef struct {
    Model* models;                 // dynamic array
    uint32_t modelCount;
    uint32_t modelCapacity;

    InstanceData* instances;       // still one big buffer for the GPU
    uint32_t instanceCapacity;
    uint32_t instanceCount;        // total live instances across ALL models

    VkBuffer instanceBuffer;
    VkDeviceMemory instanceMemory;
    VkDeviceSize instanceBufferSize;
} ModelSystem;

ModelSystem g_model_system;
VkPipelineLayout modelPipelineLayout;
VkDescriptorSet modelDescriptorSet;
VkDescriptorSetLayout modelDescriptorSetLayout;
VkDescriptorPool modelDescriptorPool;

void init_model_system(void);
void load_model(const char* glb_path);
Model* find_or_load_model(const char* name);     // returns Model*
void add_model_instance(const char* model_name, const float transform[16], const float color[4]);
void draw_models(VkCommandBuffer cmd);          // one draw call for ALL instances
void update_model_instances(void);              // upload to GPU (call inside draw_models)
void destroy_model(Model* model);
void cleanup_model_system(void);

void create_model_descriptors(void);
void update_model_descriptor(void);