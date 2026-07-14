#pragma once
#include "io/model_system.h"
#include "core/params/params.h"

void export_project_binary(const char* output_path);
bool load_project_binary(const char* path);