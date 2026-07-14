#pragma once

typedef struct {
    float model[16];
    float color[4];
    
    // CPU-only bookkeeping data (Never sent to the GPU)
    uint32_t model_index;   
    float bounding_center[3]; 
    float bounding_radius;  
} CPUInstanceData;