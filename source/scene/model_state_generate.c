#include "pch.h"
#include "model_state_generate.h"

static uint32_t select_random_model_index(const SpawnableModel* pool, uint32_t pool_count) {
    float total_weight = 0.0f;
    for (uint32_t i = 0; i < pool_count; i++) total_weight += pool[i].weight;

    float roll = rand_range(0.0f, total_weight);
    float cumulative = 0.0f;

    for (uint32_t i = 0; i < pool_count; i++) {
        cumulative += pool[i].weight;
        if (roll <= cumulative) {
            return i; 
        }
    }
    return 0;
}

void generate_procedural_world_advanced(
    uint32_t target_instances, 
    const SpawnableModel* model_pool, 
    uint32_t pool_count,
    const GenerationConstraints* constraints
) {
    LOGI("Executing scaled generation sequence for %u instances across %u assets...", target_instances, pool_count);

    // 1. Resolve and cache all global tracking indices upfront to avoid frame-time lookups
    uint32_t* cached_system_indices = malloc(pool_count * sizeof(uint32_t));
    for (uint32_t m = 0; m < pool_count; m++) {
        Model* master_model = find_or_load_model(model_pool[m].glb_path);
        cached_system_indices[m] = 0;
        if (master_model) {
            for (uint32_t idx = 0; idx < g_model_system.modelCount; idx++) {
                if (&g_model_system.models[idx] == master_model) {
                    cached_system_indices[m] = idx;
                    break;
                }
            }
        }
    }

    uint32_t dynamic_attempts = 0;
    uint32_t successful_spawns = 0;
    
    // Safety break to prevent infinite execution loop under dense spatial constraints
    const uint32_t max_total_attempts = target_instances * 5; 

    while (successful_spawns < target_instances && dynamic_attempts < max_total_attempts) {
        dynamic_attempts++;

        // Step A: Pick a target asset based on natural weighting scales
        uint32_t pool_idx = select_random_model_index(model_pool, pool_count);
        uint32_t resolved_model_idx = cached_system_indices[pool_idx];

        // Step B: Calculate candidate coordinates inside the box constraints
        float tx = rand_range(constraints->min_bounds[0], constraints->max_bounds[0]);
        float ty = rand_range(constraints->min_bounds[1], constraints->max_bounds[1]);
        float tz = rand_range(constraints->min_bounds[2], constraints->max_bounds[2]);

        // Step C: Evaluate Constraints
        if (constraints->lock_y_to_ground) {
            // ty = sample_terrain_height_at(tx, tz); 
            // if (sample_terrain_slope(tx, tz) > constraints->max_slope) continue; // Reject!
        }

        // Proximity/Separation constraint check (Simplistic grid or history evaluation)
        bool spatial_invalid = false;
        /*
        for (uint32_t s = 0; s < g_model_system.instanceCount; s++) {
            float dx = g_model_system.instances[s].bounding_center[0] - tx;
            float dz = g_model_system.instances[s].bounding_center[2] - tz;
            if ((dx*dx + dz*dz) < (constraints->min_separation * constraints->min_separation)) {
                spatial_invalid = true;
                break;
            }
        }
        if (spatial_invalid) continue; // Skip to next attempt loop step
        */

        // Step D: Calculate TRS transforms if candidate passes validation checks
        float rot_y = rand_range(0.0f, 2.0f * 3.14159265f);
        float scale = rand_range(constraints->min_scale, constraints->max_scale);

        float transform[16] = {0};
        float cos_theta = cosf(rot_y);
        float sin_theta = sinf(rot_y);

        transform[0]  = cos_theta * scale;  
        transform[2]  = -sin_theta * scale; 
        transform[5]  = scale;              
        transform[8]  = sin_theta * scale;  
        transform[10] = cos_theta * scale;  
        
        transform[12] = tx;  
        transform[13] = ty;                            
        transform[14] = tz; 
        transform[15] = 1.0f;                            

        float color[4] = {
            0.5f + (tx / 1000.0f), 
            0.5f + (ty / 200.0f), 
            0.5f + (tz / 1000.0f), 
            1.0f
        };

        // Step E: Write directly to memory buffer
        uint32_t inst_idx = g_model_system.instanceCount;
        if (inst_idx < g_model_system.instanceCapacity) {
            CPUInstanceData* inst = &g_model_system.instances[inst_idx];
            memcpy(inst->model, transform, sizeof(float) * 16);
            memcpy(inst->color, color, sizeof(float) * 4);
            
            inst->model_index = resolved_model_idx;
            
            inst->bounding_center[0] = tx;
            inst->bounding_center[1] = ty;
            inst->bounding_center[2] = tz;
            inst->bounding_radius    = model_pool[pool_idx].base_radius * scale;
            
            g_model_system.instanceCount++;
            successful_spawns++;
        }
    }

    free(cached_system_indices);
    LOGI("Procedural generation complete. Placed %u/%u instances.", successful_spawns, target_instances);
}

void generate_procedural_template_world(uint32_t target_instances)
{
	float fallback_local_center[3] = {0.0f, 0.0f, 0.0f};
    float fallback_local_radius = 5.0f;

    if (strcmp(g_current_project_name, g_project_templates[0][0]) == 0) {
		SpawnableModel temple_assets[] = {
	        { "../../assets/meshes/cs_goddess_statue_opt.glb", 0.1f,  5.0f },
	        { "../../assets/meshes/cs_face_opt.glb",       0.6f,  3.5f },
	        { "../../assets/meshes/cs_goddess_statue_opt.glb",      0.3f,  1.2f }
	    };

	    // Define the boundaries and behavioral properties
	    GenerationConstraints gothic_arena_rules = {
	        .min_bounds = {-500.0f, -20.0f, -600.0f},
	        .max_bounds = { 500.0f,  50.0f, -50.0f},
	        .min_scale = 0.5f,
	        .max_scale = 2.0f,
	        .min_separation = 6.0f,
	        .lock_y_to_ground = false
	    };

	    generate_procedural_world_advanced(
	        target_instances, 
	        temple_assets, 
	        sizeof(temple_assets) / sizeof(temple_assets[0]), 
	        &gothic_arena_rules
	    );
    }
    // --- TEMPLATE MATCHING: 2D Sprite Project ---
    else if (strcmp(g_current_project_name, g_project_templates[1][0]) == 0) {
        LOGI("Initializing Ildz (2D Sprite Template) workspace configurations...");
        // Add 2D procedural logic setups here later
    }
    // --- TEMPLATE MATCHING: Pure 2D Screen Shader ---
    else if (strcmp(g_current_project_name, g_project_templates[2][0]) == 0) {
        LOGI("Initializing Pure Shader (2D Shader Template) configurations...");
        // Add single-quad presentation generation here later
    }
    // --- TEMPLATE MATCHING: Audio Visualizer ---
    else if (strcmp(g_current_project_name, g_project_templates[3][0]) == 0) {
        LOGI("Initializing Christine-AV (Audio Visualizer Template) frequencies array maps...");
        // Add specific reactive node matrices here later
    }
    else {
        LOGE("Unknown project directory structure target: '%s'.", g_current_project_name);
    }
}