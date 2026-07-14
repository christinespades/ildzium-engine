#pragma once
#include "io/project_binary.h"
#include "core/math.h"
#include "rendering/shaders_webgpu.h"
#include "scene/lights_webgpu.h"
#include "scene/model_instance_webgpu.h"
#include "scene/model_wgsl.h"
#include "model_state.h"

typedef struct {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
} Vertex3D;

void init_model_system(void);
void load_model(const char* glb_path);
Model* find_or_load_model(const char* name);     // returns Model*
void update_spatial_streaming(float cam_x, float cam_y, float cam_z, float stream_radius);
void export_project_binary(const char* output_path);
void destroy_model(Model* model);
void cleanup_model_system(void);
bool load_project_binary(const char* path);
void on_chunk_binary_received(emscripten_fetch_t *fetch) 