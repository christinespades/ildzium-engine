#pragma once
#include "io/mesh_geometry.h"

#define MAX_LOD_LEVELS 4

// Flattened index helper macro
#define LOD_SLOT(model_idx, lod_idx) (((model_idx) * MAX_LOD_LEVELS) + (lod_idx))

// Track counters for every model's distinct LOD levels
static uint32_t* g_model_lod_visible_counts = NULL; // Dimension: modelCount * MAX_LOD_LEVELS
static uint32_t* g_model_lod_gpu_offsets    = NULL; // Dimension: modelCount * MAX_LOD_LEVELS

typedef struct {
    char* name;
    MeshGeometry mesh_levels[MAX_LOD_LEVELS]; 
    uint32_t lod_count;
    uint32_t instanceOffset;
    uint32_t instanceCount;
    bool isLoaded;
    
    // Cache the model geometry's intrinsic bounding bounds
    float local_center[3];
    float local_radius;
} Model;