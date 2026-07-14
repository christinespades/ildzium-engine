#include "pch.h"
#include "model_vk_load.h"
#define CGLTF_IMPLEMENTATION
#define CGLTF_MESHOPT_DECODE 1
#define MESHOPTIMIZER_IMPLEMENTATION
#include "cgltf.h"

extern VkDevice vk_device;

void upload_data_to_master_vertex_buffer(void* data, VkDeviceSize size, VkDeviceSize offset) 
{
    // Offset the stored base pointer by our calculated byte offset
    char* target_dest = (char*)g_model_system.masterVertexMappedData + offset;
    memcpy(target_dest, data, size);
}

void upload_data_to_master_index_buffer(void* data, VkDeviceSize size, VkDeviceSize offset) 
{
    // Offset the stored base pointer by our calculated byte offset
    char* target_dest = (char*)g_model_system.masterIndexMappedData + offset;
    memcpy(target_dest, data, size);
}

Model* store_and_free_model(
    Model* model,
    const char* base_asset_name,
    uint32_t lod_level,
    MeshGeometry mesh_geometry,
    uint32_t vertex_count,
    Vertex3D* vertices,
    uint32_t* indices,
    float* positions,
    float* normals,
    float* uvs,
    cgltf_data* data
)
{
    // Allocate a new model tracking instance entry only on the base pass (LOD 0)
    if (!model) {
        if (g_model_system.modelCount >= g_model_system.modelCapacity) {
            g_model_system.modelCapacity = g_model_system.modelCapacity ? g_model_system.modelCapacity * 2 : 8;
            g_model_system.models = realloc(g_model_system.models, g_model_system.modelCapacity * sizeof(Model));
        }

        model = &g_model_system.models[g_model_system.modelCount++];
        memset(model, 0, sizeof(*model));
        model->name = _strdup(base_asset_name); // Keep base tracking name (e.g., "cs_goddess_statue")
    }

    // Assign the sub-allocation metadata fields to our specific target slot
    model->mesh_levels[lod_level] = mesh_geometry;
    if (lod_level >= model->lod_count) {
        model->lod_count = lod_level + 1;
    }

    // Bounding metrics are calculated from the crisp LOD0 high-detail version only
    if (lod_level == 0) {
        float min_x = 1e6f, min_y = 1e6f, min_z = 1e6f;
        float max_x = -1e6f, max_y = -1e6f, max_z = -1e6f;
        for (uint32_t v = 0; v < vertex_count; v++) {
            if (vertices[v].x < min_x) min_x = vertices[v].x;
            if (vertices[v].y < min_y) min_y = vertices[v].y;
            if (vertices[v].z < min_z) min_z = vertices[v].z;
            if (vertices[v].x > max_x) max_x = vertices[v].x;
            if (vertices[v].y > max_y) max_y = vertices[v].y;
            if (vertices[v].z > max_z) max_z = vertices[v].z;
        }
        model->local_center[0] = (min_x + max_x) * 0.5f;
        model->local_center[1] = (min_y + max_y) * 0.5f;
        model->local_center[2] = (min_z + max_z) * 0.5f;

        float max_r2 = 0.0f;
        for (uint32_t v = 0; v < vertex_count; v++) {
            float dx = vertices[v].x - model->local_center[0];
            float dy = vertices[v].y - model->local_center[1];
            float dz = vertices[v].z - model->local_center[2];
            float dist2 = dx*dx + dy*dy + dz*dz;
            if (dist2 > max_r2) max_r2 = dist2;
        }
        model->local_radius = sqrtf(max_r2);
    }

    // Clean temporary parse footprints
    free(vertices);
    free(indices);
    free(positions);
    free(normals);
    free(uvs);
    cgltf_free(data);

    return model;
}

Model* load_single_glb_to_master_slot(const char* glb_path, const char* base_name, uint32_t lod_level, Model* existing_model)
{
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, glb_path, &data);
    if (result != cgltf_result_success) return NULL;

    result = cgltf_load_buffers(&options, data, glb_path);
    if (result != cgltf_result_success) {
        cgltf_free(data);
        return NULL;
    }

    cgltf_primitive* prim = &data->meshes[0].primitives[0];
    float *positions = NULL, *normals = NULL, *uvs = NULL;
    cgltf_size pos_count = 0, norm_count = 0, uv_count = 0;

    for (cgltf_size i = 0; i < prim->attributes_count; i++) {
        cgltf_attribute* attr = &prim->attributes[i];
        if (attr->type == cgltf_attribute_type_position) {
            pos_count = cgltf_accessor_unpack_floats(attr->data, NULL, 0);
            positions = (float*)malloc(pos_count * sizeof(float));
            cgltf_accessor_unpack_floats(attr->data, positions, pos_count);
        } else if (attr->type == cgltf_attribute_type_normal) {
            norm_count = cgltf_accessor_unpack_floats(attr->data, NULL, 0);
            normals = (float*)malloc(norm_count * sizeof(float));
            cgltf_accessor_unpack_floats(attr->data, normals, norm_count);
        } else if (attr->type == cgltf_attribute_type_texcoord) {
            uv_count = cgltf_accessor_unpack_floats(attr->data, NULL, 0);
            uvs = (float*)malloc(uv_count * sizeof(float));
            cgltf_accessor_unpack_floats(attr->data, uvs, uv_count);
        }
    }

    uint32_t* indices = NULL;
    cgltf_size index_count = 0;
    if (prim->indices) {
        index_count = prim->indices->count;
        indices = (uint32_t*)malloc(index_count * sizeof(uint32_t));
        cgltf_accessor_unpack_indices(prim->indices, indices, sizeof(uint32_t), index_count);
    }

    uint32_t vertex_count = (uint32_t)(pos_count / 3);
    Vertex3D* vertices = (Vertex3D*)malloc(vertex_count * sizeof(Vertex3D));

    for (uint32_t i = 0; i < vertex_count; i++) {
        vertices[i].x = positions[i*3 + 0];
        vertices[i].y = positions[i*3 + 1];
        vertices[i].z = positions[i*3 + 2];
        vertices[i].nx = normals ? normals[i*3 + 0] : 0.0f;
        vertices[i].ny = normals ? normals[i*3 + 1] : 0.0f;
        vertices[i].nz = normals ? normals[i*3 + 2] : 1.0f;
        vertices[i].u = uvs ? uvs[i*2 + 0] : 0.0f;
        vertices[i].v = uvs ? uvs[i*2 + 1] : 0.0f;
    }

    // --- MASTER BUFFER ALLOCATION MECHANISM ---
    MeshGeometry mesh = {0};
    mesh.vertexBuffer = g_model_system.masterVertexBuffer;
    mesh.indexBuffer  = g_model_system.masterIndexBuffer;
    mesh.indexCount   = (uint32_t)index_count;

    // Track offsets relative to your master buffer lengths
    mesh.vertexOffset = g_model_system.masterVertexTailCount;
    mesh.firstIndex   = g_model_system.masterIndexTailCount;

    // Map and upload vertex block data directly to the structural tail segment offsets
    upload_data_to_master_vertex_buffer(vertices, vertex_count * sizeof(Vertex3D), mesh.vertexOffset * sizeof(Vertex3D));
    if (index_count > 0) {
        upload_data_to_master_index_buffer(indices, index_count * sizeof(uint32_t), mesh.firstIndex * sizeof(uint32_t));
    }

    // Advance your global tracking metrics forward
    g_model_system.masterVertexTailCount += vertex_count;
    g_model_system.masterIndexTailCount  += (uint32_t)index_count;

    return store_and_free_model(
        existing_model, base_name, lod_level, mesh, vertex_count, vertices, indices, positions, normals, uvs, data
    );
}

Model* find_or_load_model(const char* base_glb_path)
{
    // Declare tracking pointers at the top scope
    Model* target_model = NULL;

    char clean_base[256];
    strncpy(clean_base, base_glb_path, sizeof(clean_base));
    char* ext_point = strstr(clean_base, "_opt.glb");
    if (!ext_point) ext_point = strstr(clean_base, ".glb");
    if (ext_point) *ext_point = '\0'; 

    // 1. Check lookup trackers using the standardized clean string name
    for (uint32_t i = 0; i < g_model_system.modelCount; ++i) {
        if (strcmp(g_model_system.models[i].name, clean_base) == 0) {
            // Only return early if this model entry has ACTUALLY been populated!
            if (g_model_system.models[i].isLoaded && g_model_system.models[i].lod_count > 0) {
                return &g_model_system.models[i]; 
            }
            // Reuse the existing allocated slot from the binary stub
            target_model = &g_model_system.models[i];
            break;
        }
    }

    // Target variants array mapping straight to LOD IDs
    char variant_paths[4][512];
    snprintf(variant_paths[0], 512, "%s_opt.glb",  clean_base);
    snprintf(variant_paths[1], 512, "%s_lod1.glb", clean_base);
    snprintf(variant_paths[2], 512, "%s_lod2.glb", clean_base);
    snprintf(variant_paths[3], 512, "%s_lod3.glb", clean_base);

    // If target_model is still NULL after cache check, it's a completely new runtime asset allocation
    bool is_new_allocation = (target_model == NULL);

    // 3. Sequentially process arrays into unified slot indexes
    for (uint32_t l = 0; l < 4; l++) {
        FILE* f_test = fopen(variant_paths[l], "rb");
        if (!f_test) {
            if (l == 0) {
                LOGE("Critical error: Base high-poly asset could not be found at: %s", variant_paths[0]);
                return NULL;
            }
            continue; 
        }
        fclose(f_test);

        // --- FIX: Pass target_model directly. Once LOD 0 instantiates it, 
        // LOD 1, 2, and 3 will reuse it instead of generating new entries. ---
        target_model = load_single_glb_to_master_slot(variant_paths[l], clean_base, l, target_model);
    }

    if (target_model) {
        target_model->isLoaded = true;
        LOGI("Successfully registered integrated multi-LOD asset: %s (%u variants configured)", 
             clean_base, target_model->lod_count);
    }

    return target_model;
}