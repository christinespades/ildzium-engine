#include "pch.h"
#include "model_state.h"

bool load_model_state(const char* project_bin_path) 
{
    g_model_system.models = NULL;
    g_model_system.modelCount = 0;
    g_model_system.modelCapacity = 0;

    g_model_system.instances = NULL;
    g_model_system.instanceCount = 0;
    g_model_system.instanceCapacity = get_param_float(PARAM_RENDER_MAX_MODEL_INSTANCES);
    g_model_system.instances = malloc(g_model_system.instanceCapacity * sizeof(CPUInstanceData));

    g_model_system.visibleCount = 0;
    g_model_system.visibleInstancesCPU = malloc(g_model_system.instanceCapacity * sizeof(InstanceData));
    bool binary_loaded = false;

    // Try loading the pre-baked project state
    FILE* check_file = fopen(project_bin_path, "rb");
    if (check_file) {
        fclose(check_file);
        binary_loaded = load_project_binary(project_bin_path);
    }
  	if (binary_loaded) {
  		return true;
  	} else {
  		return false;
  	}
}