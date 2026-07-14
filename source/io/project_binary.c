#include "pch.h"
#ifndef __EMSCRIPTEN__
    #include "io/project_binary.h"

    static int write_u32_le(FILE* f, uint32_t v) {
        uint8_t b[4];
        b[0] = (uint8_t)(v & 0xFF);
        b[1] = (uint8_t)((v >> 8) & 0xFF);
        b[2] = (uint8_t)((v >> 16) & 0xFF);
        b[3] = (uint8_t)((v >> 24) & 0xFF);
        return fwrite(b, 1, 4, f) == 4;
    }

    static int write_f32_le(FILE* f, float v) {
        uint32_t u;
        memcpy(&u, &v, sizeof(u));
        return write_u32_le(f, u);
    }

    static int write_bytes(FILE* f, const void* data, size_t size) {
        return size == 0 || fwrite(data, 1, size, f) == size;
    }

    static void free_buckets(CPUInstanceData** buckets, uint32_t total_buckets) {
        if (!buckets) return;
        for (uint32_t i = 0; i < total_buckets; i++) free(buckets[i]);
        free(buckets);
    }

    static int write_cpu_instance_data(FILE* file, const CPUInstanceData* inst)
    {
        for (int i = 0; i < 16; i++) if (!write_f32_le(file, inst->model[i])) return 0;
        for (int i = 0; i < 4; i++) if (!write_f32_le(file, inst->color[i])) return 0;
        if (!write_u32_le(file, inst->model_index)) return 0;
        for (int i = 0; i < 3; i++) if (!write_f32_le(file, inst->bounding_center[i])) return 0;
        return write_f32_le(file, inst->bounding_radius);
    }

    static void destroy_project_binary_state(void)
    {
        if (g_model_system.models) {
            for (uint32_t i = 0; i < g_model_system.modelCount; i++) {
                free(g_model_system.models[i].name);
                g_model_system.models[i].name = NULL;
            }
            free(g_model_system.models);
            g_model_system.models = NULL;
        }

        free(g_model_system.model_lod_visible_counts);
        g_model_system.model_lod_visible_counts = NULL;

        free(g_model_system.model_lod_gpu_offsets);
        g_model_system.model_lod_gpu_offsets = NULL;

        free(g_model_system.chunks);
        g_model_system.chunks = NULL;

        free(g_model_system.chunks);
        g_model_system.chunks = NULL;

        free(g_model_system.instances);
        g_model_system.instances = NULL;

        free(g_model_system.visibleInstancesCPU);
        g_model_system.visibleInstancesCPU = NULL;

        g_model_system.modelCount = 0;
        g_model_system.chunkCount = 0;
        g_model_system.instanceCount = 0;
        g_model_system.instanceCapacity = 0;
    }

    void export_project_binary(const char* output_path)
    {
        FILE* file = fopen(output_path, "wb");
        if (!file) {
            LOGE("Failed to open file for binary export: %s", output_path);
            return;
        }

        int ok = 1;
        uint32_t magic = 0x494C445A; /* "ILDZ" */
        uint32_t version = 1;
        uint32_t model_count = g_model_system.modelCount;
        uint32_t instance_count = g_model_system.instanceCount;
        uint32_t total_buckets = MAX_CHUNKS_X * MAX_CHUNKS_Z;

        uint32_t* bucket_counts = NULL;
        uint32_t* bucket_capacities = NULL;
        CPUInstanceData** buckets = NULL;

        ok &= write_u32_le(file, magic);
        ok &= write_u32_le(file, version);
        ok &= write_u32_le(file, model_count);

        for (uint32_t i = 0; i < model_count && ok; i++) {
            uint32_t name_len = (uint32_t)strlen(g_model_system.models[i].name);
            ok &= write_u32_le(file, name_len);
            ok &= write_bytes(file, g_model_system.models[i].name, name_len);
            ok &= write_f32_le(file, g_model_system.models[i].local_center[0]);
            ok &= write_f32_le(file, g_model_system.models[i].local_center[1]);
            ok &= write_f32_le(file, g_model_system.models[i].local_center[2]);
            ok &= write_f32_le(file, g_model_system.models[i].local_radius);
        }

        if (!ok) {
            LOGE("Binary export failed.");
            fclose(file);
            return;
        }

        if (instance_count == 0) {
            ok &= write_u32_le(file, 0);
            if (!ok) LOGE("Binary export failed.");
            else LOGI("Successfully baked world spatial index profile!");
            fclose(file);
            return;
        }

        float min_x = g_model_system.instances[0].bounding_center[0];
        float max_x = min_x;
        float min_z = g_model_system.instances[0].bounding_center[2];
        float max_z = min_z;

        for (uint32_t i = 1; i < instance_count; i++) {
            float x = g_model_system.instances[i].bounding_center[0];
            float z = g_model_system.instances[i].bounding_center[2];
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (z < min_z) min_z = z;
            if (z > max_z) max_z = z;
        }

        float size_x = max_x - min_x;
        float size_z = max_z - min_z;
        if (size_x <= 0.000001f) size_x = 1.0f;
        if (size_z <= 0.000001f) size_z = 1.0f;

        bucket_counts = calloc(total_buckets, sizeof(uint32_t));
        bucket_capacities = calloc(total_buckets, sizeof(uint32_t));
        buckets = calloc(total_buckets, sizeof(CPUInstanceData*));

        if (!bucket_counts || !bucket_capacities || !buckets) {
            LOGE("Out of memory during export.");
            goto cleanup;
        }

        for (uint32_t i = 0; i < instance_count; i++) {
            CPUInstanceData inst = g_model_system.instances[i];

            int ix = (int)(((inst.bounding_center[0] - min_x) / size_x) * MAX_CHUNKS_X);
            int iz = (int)(((inst.bounding_center[2] - min_z) / size_z) * MAX_CHUNKS_Z);

            if (ix < 0) ix = 0;
            if (ix >= MAX_CHUNKS_X) ix = MAX_CHUNKS_X - 1;
            if (iz < 0) iz = 0;
            if (iz >= MAX_CHUNKS_Z) iz = MAX_CHUNKS_Z - 1;

            uint32_t b_idx = (uint32_t)iz * MAX_CHUNKS_X + (uint32_t)ix;

            if (bucket_counts[b_idx] >= bucket_capacities[b_idx]) {
                uint32_t new_cap = bucket_capacities[b_idx] ? bucket_capacities[b_idx] * 2 : 16;
                CPUInstanceData* new_buf = realloc(buckets[b_idx], new_cap * sizeof(CPUInstanceData));
                if (!new_buf) {
                    LOGE("Out of memory while growing bucket.");
                    goto cleanup;
                }
                buckets[b_idx] = new_buf;
                bucket_capacities[b_idx] = new_cap;
            }

            buckets[b_idx][bucket_counts[b_idx]++] = inst;
        }

        uint32_t valid_chunks = 0;
        for (uint32_t i = 0; i < total_buckets; i++) {
            if (bucket_counts[i] > 0) valid_chunks++;
        }

        ok &= write_u32_le(file, valid_chunks);

        uint32_t running_offset = 0;

        for (uint32_t i = 0; i < total_buckets && ok; i++) {
            if (bucket_counts[i] == 0) continue;

            float c_min_x = 1e30f, c_max_x = -1e30f;
            float c_min_y = 1e30f, c_max_y = -1e30f;
            float c_min_z = 1e30f, c_max_z = -1e30f;

            for (uint32_t j = 0; j < bucket_counts[i]; j++) {
                float x = buckets[i][j].bounding_center[0];
                float y = buckets[i][j].bounding_center[1];
                float z = buckets[i][j].bounding_center[2];
                float r = buckets[i][j].bounding_radius;

                if (x - r < c_min_x) c_min_x = x - r;
                if (x + r > c_max_x) c_max_x = x + r;
                if (y - r < c_min_y) c_min_y = y - r;
                if (y + r > c_max_y) c_max_y = y + r;
                if (z - r < c_min_z) c_min_z = z - r;
                if (z + r > c_max_z) c_max_z = z + r;
            }

            float center_x = (c_min_x + c_max_x) * 0.5f;
            float center_y = (c_min_y + c_max_y) * 0.5f;
            float center_z = (c_min_z + c_max_z) * 0.5f;

            float dx = c_max_x - center_x;
            float dy = c_max_y - center_y;
            float dz = c_max_z - center_z;

            float radius = sqrtf(dx * dx + dy * dy + dz * dz);
            uint32_t instance_offset = running_offset;
            uint32_t instance_count_in_chunk = bucket_counts[i];

            ok &= write_f32_le(file, center_x);
            ok &= write_f32_le(file, center_y);
            ok &= write_f32_le(file, center_z);
            ok &= write_f32_le(file, radius);
            ok &= write_u32_le(file, instance_offset);
            ok &= write_u32_le(file, instance_count_in_chunk);

            running_offset += instance_count_in_chunk;
        }

        for (uint32_t i = 0; i < total_buckets && ok; i++) {
            if (bucket_counts[i] == 0) continue;

            for (uint32_t j = 0; j < bucket_counts[i]; j++) {
                ok &= write_cpu_instance_data(file, &buckets[i][j]);
            }
        }

        if (!ok) {
            LOGE("Binary export failed while writing file.");
        } else {
            LOGI("Successfully baked world spatial index profile!");
        }

    cleanup:
        free(bucket_counts);
        free(bucket_capacities);
        free_buckets(buckets, total_buckets);
        fclose(file);
    }

    static bool read_u32_le(FILE* f, uint32_t* out)
    {
        uint8_t b[4];
        if (fread(b, 1, 4, f) != 4) return false;
        *out = ((uint32_t)b[0]) |
               ((uint32_t)b[1] << 8) |
               ((uint32_t)b[2] << 16) |
               ((uint32_t)b[3] << 24);
        return true;
    }

    static bool read_f32_le(FILE* f, float* out)
    {
        uint32_t u;
        if (!read_u32_le(f, &u)) return false;
        memcpy(out, &u, sizeof(u));
        return true;
    }

    static bool read_bytes(FILE* f, void* dst, size_t size)
    {
        return size == 0 || fread(dst, 1, size, f) == size;
    }

    // Force-verify that your binary parsing utility matches this exact sequential order:
    bool read_cpu_instance_data(FILE* file, CPUInstanceData* out_data) {
        // 1. Read the 16 floats for the transform matrix
        for (int i = 0; i < 16; i++) {
            if (!read_f32_le(file, &out_data->model[i])) return false;
        }
        // 2. Read the 4 floats for color
        for (int i = 0; i < 4; i++) {
            if (!read_f32_le(file, &out_data->color[i])) return false;
        }
        // 3. Read internal meta properties explicit widths
        if (!read_u32_le(file, &out_data->model_index)) return false;
        if (!read_f32_le(file, &out_data->bounding_center[0])) return false;
        if (!read_f32_le(file, &out_data->bounding_center[1])) return false;
        if (!read_f32_le(file, &out_data->bounding_center[2])) return false;
        if (!read_f32_le(file, &out_data->bounding_radius)) return false;

        return true;
    }

    bool load_project_binary(const char* path)
    {
        FILE* file = fopen(path, "rb");
        if (!file) return false;

        uint32_t magic = 0;
        uint32_t version = 0;
        uint32_t model_count = 0;
        uint32_t chunk_count = 0;
        uint32_t total_instances_count = 0;
        bool ok = true;

        if (!read_u32_le(file, &magic) || !read_u32_le(file, &version)) {
            ok = false;
            goto cleanup;
        }

        if (magic != 0x494C445A) {
            LOGE("Invalid engine project file header magic alignment!");
            ok = false;
            goto cleanup;
        }

        if (version != 1) {
            LOGE("Unsupported project file version: %u", version);
            ok = false;
            goto cleanup;
        }

        if (!read_u32_le(file, &model_count)) {
            ok = false;
            goto cleanup;
        }

        destroy_project_binary_state();
        g_model_system.modelCount = model_count;
        g_model_system.models = calloc(model_count, sizeof(Model));
        if (!g_model_system.models && model_count > 0) {
            ok = false;
            goto cleanup;
        }

        for (uint32_t i = 0; i < model_count; i++) {
            uint32_t name_len = 0;

            if (!read_u32_le(file, &name_len)) {
                ok = false;
                goto cleanup;
            }

            g_model_system.models[i].name = malloc((size_t)name_len + 1);
            if (!g_model_system.models[i].name) {
                ok = false;
                goto cleanup;
            }

            if (!read_bytes(file, g_model_system.models[i].name, name_len)) {
                ok = false;
                goto cleanup;
            }
            g_model_system.models[i].name[name_len] = '\0';

            if (!read_f32_le(file, &g_model_system.models[i].local_center[0]) ||
                !read_f32_le(file, &g_model_system.models[i].local_center[1]) ||
                !read_f32_le(file, &g_model_system.models[i].local_center[2]) ||
                !read_f32_le(file, &g_model_system.models[i].local_radius)) {
                ok = false;
                goto cleanup;
            }

            g_model_system.models[i].lod_count = 0;
            g_model_system.models[i].isLoaded = false;
            g_model_system.models[i].instanceCount = 0;
            g_model_system.models[i].instanceOffset = 0;

            for (uint32_t l = 0; l < MAX_LOD_LEVELS; l++) {
                g_model_system.models[i].mesh_levels[l].vertexBuffer = VK_NULL_HANDLE;
                g_model_system.models[i].mesh_levels[l].indexBuffer  = VK_NULL_HANDLE;
                g_model_system.models[i].mesh_levels[l].indexCount   = 0;
                g_model_system.models[i].mesh_levels[l].firstIndex   = 0;
                g_model_system.models[i].mesh_levels[l].vertexOffset = 0;
            }
        }

        // Now that modelCount is officially set, allocate the visibility tracking buffers
        g_model_system.model_lod_visible_counts = realloc(g_model_system.model_lod_visible_counts, g_model_system.modelCount * MAX_LOD_LEVELS * sizeof(uint32_t));
        g_model_system.model_lod_gpu_offsets    = realloc(g_model_system.model_lod_gpu_offsets,    g_model_system.modelCount * MAX_LOD_LEVELS * sizeof(uint32_t));

        memset(g_model_system.model_lod_visible_counts, 0, g_model_system.modelCount * MAX_LOD_LEVELS * sizeof(uint32_t));
        memset(g_model_system.model_lod_gpu_offsets,    0, g_model_system.modelCount * MAX_LOD_LEVELS * sizeof(uint32_t));

        if (!read_u32_le(file, &chunk_count)) {
            ok = false;
            goto cleanup;
        }

        g_model_system.chunkCount = chunk_count;
        g_model_system.chunks = calloc(chunk_count, sizeof(SpatialChunk));
        if (!g_model_system.chunks && chunk_count > 0) {
            ok = false;
            goto cleanup;
        }

        for (uint32_t i = 0; i < chunk_count; i++) {
            if (!read_f32_le(file, &g_model_system.chunks[i].center[0]) ||
                !read_f32_le(file, &g_model_system.chunks[i].center[1]) ||
                !read_f32_le(file, &g_model_system.chunks[i].center[2]) ||
                !read_f32_le(file, &g_model_system.chunks[i].radius) ||
                !read_u32_le(file, &g_model_system.chunks[i].instanceOffset) ||
                !read_u32_le(file, &g_model_system.chunks[i].instanceCount)) {
                ok = false;
                goto cleanup;
            }

            g_model_system.chunks[i].isLoaded = true;
            total_instances_count += g_model_system.chunks[i].instanceCount;
        }

        if (total_instances_count > g_model_system.instanceCapacity) {
            CPUInstanceData* new_instances = realloc(g_model_system.instances,
                                                     total_instances_count * sizeof(CPUInstanceData));
            if (!new_instances && total_instances_count > 0) {
                ok = false;
                goto cleanup;
            }
            g_model_system.instances = new_instances;

            InstanceData* new_visible = realloc(g_model_system.visibleInstancesCPU,
                                                total_instances_count * sizeof(InstanceData));
            if (!new_visible && total_instances_count > 0) {
                ok = false;
                goto cleanup;
            }
            g_model_system.visibleInstancesCPU = new_visible;
            g_model_system.instanceCapacity = total_instances_count;
        }

        g_model_system.instanceCount = total_instances_count;

        for (uint32_t i = 0; i < total_instances_count; i++) {
            if (!read_cpu_instance_data(file, &g_model_system.instances[i])) {
                ok = false;
                goto cleanup;
            }
        }

    cleanup:
        fclose(file);

        if (!ok) {
            destroy_project_binary_state();
            return false;
        }

        LOGI("Successfully initialized %u spatial chunks tracking %u instances from binary.", g_model_system.chunkCount, g_model_system.instanceCount);
        return true;
    }
#endif