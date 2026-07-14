#pragma once
#include "core/platform.h"

#define MAX_PROJECT_NAME 128
#define PROJECTS_DIR "C:\\Users\\Public\\Downloads\\000github\\ildzium-engine\\projects"
#define DEFAULT_PROJECT_DIR_NAME "ildrathia"
#define DEFAULT_PROJECT_NAME "Ildrathia (3D Template)"

extern char g_current_project_name[MAX_PROJECT_NAME];
extern bool g_is_rename_mode;
extern bool g_show_project_modal;
extern char g_modal_old_name[MAX_PROJECT_NAME];
extern char g_current_project_path[512];
const char* g_project_templates[][2];

typedef struct Project {
    char name[MAX_PROJECT_NAME];
    char folder_path[512];        // full path to project folder
    uint64_t last_modified;       // for sorting "most recent"
} Project;

void project_init(void);
void project_shutdown(void);

bool project_create(const char* name);
bool project_load(const char* name);
bool project_rename(const char* old_name, const char* new_name);
bool project_delete(const char* name);
Project* project_get_list(int* out_count);
void project_refresh_list(void);
bool project_exists(const char* name);
void project_show_new_dialog(void);
void project_show_rename_dialog(const char* current_name);