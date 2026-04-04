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
    float model[16];   // 4x4 transform matrix
    float color[4];    // RGBA tint
} InstanceData;

typedef struct {
    Mesh* meshes;
    uint32_t meshCount;

    InstanceData* instances;
    uint32_t instanceCount;
    uint32_t instanceCapacity;

    // GPU dynamic instance buffer
    VkBuffer instanceBuffer;
    VkDeviceMemory instanceMemory;
    VkDeviceSize instanceBufferSize;
} Model;

Model g_model;

void init_model_system(void);
void load_model(const char* glb_path);
void add_model_instance(const float* modelMatrix, const float* color); // color can be NULL
void draw_models(VkCommandBuffer cmd);          // one draw call for ALL instances
void update_model_instances(void);              // upload to GPU (call inside draw_models)
void destroy_model(Model* model);
void cleanup_model_system(void);