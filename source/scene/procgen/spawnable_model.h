#pragma once

typedef struct {
    const char* glb_path;
    float weight;            // Relative selection weight (e.g., 0.8 for rocks, 0.05 for shrines)
    float base_radius;       // Default bounding radius before scale mutations
} SpawnableModel;