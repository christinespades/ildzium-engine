#pragma once

// Represents a bounded spatial cell (e.g., a grid cell or cluster)
typedef struct {
    float center[3];
    float radius;
    uint32_t instanceOffset; // Where its instances start in g_model_system.instances
    uint32_t instanceCount;  // Number of instances in this sector
    bool isLoaded;           // For streaming geometry packages if decoupled later
} SpatialChunk;