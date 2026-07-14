#pragma once
#include "io/project_binary.h"
#include "core/params/params.h"

extern ModelSystem g_model_system;

bool load_model_state(const char* project_bin_path);