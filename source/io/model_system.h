#pragma once
#include "io/model.h"
#include "io/cpu_instance_data.h"
#include "io/spatial_chunk.h"
#include "io/instance_data.h"

typedef struct {
    // --- Model Tracking Arena ---
    Model* models;                  // Dynamic array of structural assets
    uint32_t modelCount;
    uint32_t modelCapacity;

    // --- Core Master Geometry Arena ---
    // Instead of individual buffers per mesh, all vertex/index data goes here
    VkBuffer masterVertexBuffer;
    VkDeviceMemory masterVertexMemory;
    VkBuffer masterIndexBuffer;
    VkDeviceMemory masterIndexMemory;

    void* masterVertexMappedData;   // Stored CPU address for the vertex arena
    void* masterIndexMappedData;    // Stored CPU address for the index arena

    // Cursors tracking the raw count element tail for subsequent mesh packing
    uint32_t masterVertexTailCount; // Incremented by vertex_count per loaded mesh
    uint32_t masterIndexTailCount;  // Incremented by index_count per loaded mesh

    // --- Instance Placement Data ---
    CPUInstanceData* instances;     // Source procedural/loaded placement definitions
    uint32_t instanceCapacity;
    uint32_t instanceCount;         // Total live instances across ALL models

    // --- Spatial Subdivision ---
    SpatialChunk* chunks;
    uint32_t chunkCount;

    // --- Option B Flattened Streaming Visibility Maps ---
    // Sized as: modelCount * MAX_LOD_LEVELS
    uint32_t* model_lod_visible_counts; 
    uint32_t* model_lod_gpu_offsets;    

    // --- Double buffer scratchpad for instances visible THIS frame ---
    InstanceData* visibleInstancesCPU; 
    uint32_t visibleCount;
    VkBuffer instanceBuffer;
    VkDeviceMemory instanceMemory;
    VkDeviceSize instanceBufferSize;
    void* mappedData;
} ModelSystem;

extern ModelSystem g_model_system;