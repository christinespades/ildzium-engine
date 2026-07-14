#pragma once

typedef struct {
    float min_bounds[3];     // Spatially bound minimum placement box [X, Y, Z]
    float max_bounds[3];     // Spatially bound maximum placement box [X, Y, Z]
    float min_scale;
    float max_scale;
    float max_slope;         // Constraint: Kill execution if terrain slope calculation exceeds limit
    float min_separation;    // Constraint: Minimum distance between instances (Poisson-disk or grid jitter)
    bool lock_y_to_ground;   // Constraint: Raycast down to map geometry heights
} GenerationConstraints;