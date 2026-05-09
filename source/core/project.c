#include "pch.h"
#include "project.h"

static char current_project[MAX_PROJECT_NAME] = "Default";
static char current_project_path[512] = "";

static Project* g_projects = NULL;
static int g_project_count = 0;
bool g_show_project_modal = false;
bool g_is_rename_mode = false;
char g_modal_old_name[MAX_PROJECT_NAME] = "";

// Add these two functions if not already there
void project_show_new_dialog(void)
{
    g_show_project_modal = true;
    g_is_rename_mode = false;
    g_modal_old_name[0] = '\0';
}

void project_show_rename_dialog(const char* current_name)
{
    if (!current_name) return;
    g_show_project_modal = true;
    g_is_rename_mode = true;
    strncpy(g_modal_old_name, current_name, MAX_PROJECT_NAME - 1);
    g_modal_old_name[MAX_PROJECT_NAME - 1] = '\0';
}

static void ensure_default_project(void)
{
    if (!project_exists("Default"))
    {
        //platform_create_directory("projects/Default");
        // TODO: create default scene file, config, etc.
        printf("Created Default project");
    }
}

void project_init(void)
{
    //platform_create_directory(PROJECTS_DIR);
    ensure_default_project();
    project_refresh_list();
    project_load("Default");   // always start with Default
}

void project_shutdown(void)
{
    free(g_projects);
    g_projects = NULL;
    g_project_count = 0;
}

void project_refresh_list(void)
{
    free(g_projects);
    g_projects = NULL;
    g_project_count = 0;

    // Scan projects directory
    PlatformDir* dir = platform_open_dir(PROJECTS_DIR);
    if (!dir) return;

    PlatformDirEntry entry;
    while (platform_read_dir(dir, &entry))
    {
        if (entry.is_directory && strcmp(entry.name, ".") != 0 && strcmp(entry.name, "..") != 0)
        {
            g_projects = realloc(g_projects, (g_project_count + 1) * sizeof(Project));
            Project* p = &g_projects[g_project_count];

            strncpy(p->name, entry.name, MAX_PROJECT_NAME - 1);
            snprintf(p->folder_path, sizeof(p->folder_path), "%s/%s", PROJECTS_DIR, entry.name);
            
            // Get last modified time for sorting
            p->last_modified = platform_get_file_time(p->folder_path);
            g_project_count++;
        }
    }
    platform_close_dir(dir);

    // TODO: sort by last_modified descending (most recent first)
}

bool project_create(const char* name)
{
    if (!name || strlen(name) == 0 || project_exists(name))
        return false;

    char path[512];
    snprintf(path, sizeof(path), "%s/%s", PROJECTS_DIR, name);

    if (platform_create_directory(path))
    {
        project_refresh_list();
        return project_load(name);
    }
    return false;
}

bool project_load(const char* name)
{
    if (!project_exists(name))
        return false;

    strncpy(current_project, name, MAX_PROJECT_NAME - 1);
    snprintf(current_project_path, sizeof(current_project_path), 
             "%s/%s", PROJECTS_DIR, name);

    printf("Loaded project: %s", name);
    // TODO: Load scene, settings, assets, etc.
    return true;
}

bool project_rename(const char* old_name, const char* new_name)
{
    if (!old_name || !new_name || !project_exists(old_name) || project_exists(new_name))
        return false;

    char old_path[512], new_path[512];
    snprintf(old_path, sizeof(old_path), "%s/%s", PROJECTS_DIR, old_name);
    snprintf(new_path, sizeof(new_path), "%s/%s", PROJECTS_DIR, new_name);

    if (platform_rename(old_path, new_path))
    {
        if (strcmp(current_project, old_name) == 0)
            project_load(new_name);

        project_refresh_list();
        return true;
    }
    return false;
}

bool project_delete(const char* name)
{
    if (strcmp(name, "Default") == 0) return false; // protect default

    char path[512];
    snprintf(path, sizeof(path), "%s/%s", PROJECTS_DIR, name);

    if (platform_delete_directory(path))
    {
        if (strcmp(current_project, name) == 0)
            project_load("Default");

        project_refresh_list();
        return true;
    }
    return false;
}

bool project_exists(const char* name)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", PROJECTS_DIR, name);
    return platform_directory_exists(path);
}

const char* project_get_current_name(void) { return current_project; }
const char* project_get_current_path(void) { return current_project_path; }

Project* project_get_list(int* out_count)
{
    *out_count = g_project_count;
    return g_projects;
}