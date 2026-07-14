#include "pch.h"
#include "project.h"

char g_current_project_name[MAX_PROJECT_NAME] = DEFAULT_PROJECT_NAME;
char g_current_project_path[512] = "";

static Project* g_projects = NULL; 
static int g_project_count = 0;

bool g_show_project_modal = false;
bool g_is_rename_mode = false;
char g_modal_old_name[MAX_PROJECT_NAME] = "";
const char* g_project_templates[][2] = {
    { DEFAULT_PROJECT_NAME, DEFAULT_PROJECT_DIR_NAME },
    { "Minecraft (3D Block Template)", "minecraft" },
    { "Ildz (2D Sprite Template)", "ildz_sprite" },
    { "Pure Shader (2D Shader Template)", "pure_shader" },
    { "Christine-AV (Audio Visualizer Template)", "christine-av" }
};

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
    char default_path[512];
    snprintf(default_path, sizeof(default_path), "%s\\%s", PROJECTS_DIR, DEFAULT_PROJECT_DIR_NAME);

    if (!platform_directory_exists(default_path))
    {
        platform_create_directory(default_path);
        printf("Created Default project folder\n");
    }
}

void project_refresh_list(void)
{
    // Clean out old snapshot allocations
    free(g_projects);
    g_projects = NULL;
    g_project_count = 0;

    // 1. Pre-allocate a safe block of workspace size so realloc isn't jumping around in a loop
    int max_capacity = 16; 
    g_projects = malloc(max_capacity * sizeof(Project));
    if (!g_projects) return;

    // 2. Scan the disk folders
    PlatformDir* dir = platform_open_dir(PROJECTS_DIR);
    if (dir) 
    {
        PlatformDirEntry entry;
        while (platform_read_dir(dir, &entry))
        {
            if (entry.is_directory && strcmp(entry.name, ".") != 0 && strcmp(entry.name, "..") != 0)
            {
                if (strcmp(entry.name, "ildz_sprite") == 0 ||
                    strcmp(entry.name, "pure_shader") == 0 ||
                    strcmp(entry.name, "christine-av") == 0 ||
                    strcmp(entry.name, "ildrathia") == 0)
                {
                    continue; 
                }

                // Expand capacity dynamically if a user has many custom folders
                if (g_project_count >= max_capacity) {
                    max_capacity *= 2;
                    g_projects = realloc(g_projects, max_capacity * sizeof(Project));
                }

                Project* p = &g_projects[g_project_count];
                memset(p, 0, sizeof(Project)); // Clear struct memory to avoid junk string data

                strncpy(p->name, entry.name, MAX_PROJECT_NAME - 1);
                p->name[MAX_PROJECT_NAME - 1] = '\0';

                snprintf(p->folder_path, sizeof(p->folder_path), "%s\\%s", PROJECTS_DIR, entry.name);
                p->last_modified = platform_get_file_time(p->folder_path);
                g_project_count++;
            }
        }
        platform_close_dir(dir);
    }

    for (int i = 0; i < 4; i++) {
        if (g_project_count >= max_capacity) {
            max_capacity += 4;
            g_projects = realloc(g_projects, max_capacity * sizeof(Project));
        }

        char full_template_path[512];
        snprintf(full_template_path, sizeof(full_template_path), "%s\\%s", PROJECTS_DIR, g_project_templates[i][1]);
        
        platform_create_directory(full_template_path); 

        Project* p = &g_projects[g_project_count];
        memset(p, 0, sizeof(Project));

        strncpy(p->name, g_project_templates[i][0], MAX_PROJECT_NAME - 1);
        p->name[MAX_PROJECT_NAME - 1] = '\0';

        strncpy(p->folder_path, full_template_path, sizeof(p->folder_path) - 1);
        p->folder_path[sizeof(p->folder_path) - 1] = '\0';

        p->last_modified = 0;
        g_project_count++;
    }

    // Shrink allocation down to the exact final count size
    if (g_project_count > 0) {
        g_projects = realloc(g_projects, g_project_count * sizeof(Project));
    } else {
        free(g_projects);
        g_projects = NULL;
    }
}

void project_init(void)
{
    platform_create_directory(PROJECTS_DIR);
    ensure_default_project();

    project_refresh_list(); 
    project_load(DEFAULT_PROJECT_NAME); 
}

void project_shutdown(void)
{
    free(g_projects);
    g_projects = NULL;
    g_project_count = 0;
}

bool project_exists(const char* name)
{
    char path[512];
    // If checking for default template via pretty UI name
    if (strcmp(name, DEFAULT_PROJECT_NAME) == 0) {
        snprintf(path, sizeof(path), "%s\\%s", PROJECTS_DIR, DEFAULT_PROJECT_DIR_NAME);
    } else {
        snprintf(path, sizeof(path), "%s\\%s", PROJECTS_DIR, name);
    }
    return platform_directory_exists(path);
}

bool project_create(const char* name)
{
    if (!name || strlen(name) == 0 || project_exists(name))
        return false;

    char path[512];
    snprintf(path, sizeof(path), "%s\\%s", PROJECTS_DIR, name);

    if (platform_create_directory(path))
    {
        project_refresh_list();
        return project_load(name);
    }
    return false;
}

bool project_load(const char* name)
{
    // 1. Critical Defensive Guard: Prevent NULL dereferencing crashes
    if (!name) {
        LOGE("Project_load received a NULL project name!");
        return false;
    }

    // 2. Resolve internal path mapping safely
    if (strcmp(name, DEFAULT_PROJECT_NAME) == 0) {
        snprintf(g_current_project_path, sizeof(g_current_project_path), "%s\\%s", PROJECTS_DIR, DEFAULT_PROJECT_DIR_NAME);
    } else {
        snprintf(g_current_project_path, sizeof(g_current_project_path), "%s\\%s", PROJECTS_DIR, name);
    }

    // 3. Keep a single safe string copy
    strncpy(g_current_project_name, name, MAX_PROJECT_NAME - 1);
    g_current_project_name[MAX_PROJECT_NAME - 1] = '\0'; 

    LOGI("Loaded project: %s with path %s", name, g_current_project_path);
    return true;
}

bool project_rename(const char* old_name, const char* new_name)
{
    if (!old_name || !new_name || !project_exists(old_name) || project_exists(new_name))
        return false;
    if (strcmp(old_name, DEFAULT_PROJECT_NAME) == 0) return false; // protect default

    char old_path[512], new_path[512];
    snprintf(old_path, sizeof(old_path), "%s/%s", PROJECTS_DIR, old_name);
    snprintf(new_path, sizeof(new_path), "%s/%s", PROJECTS_DIR, new_name);

    if (platform_rename(old_path, new_path))
    {
        if (strcmp(g_current_project_name, old_name) == 0)
            project_load(new_name);

        project_refresh_list();
        return true;
    }
    return false;
}

bool project_delete(const char* name)
{
    if (strcmp(name, DEFAULT_PROJECT_NAME) == 0) return false; // protect default

    char path[512];
    snprintf(path, sizeof(path), "%s/%s", PROJECTS_DIR, name);

    if (platform_delete_directory(path))
    {
        if (strcmp(g_current_project_name, name) == 0)
            project_load(DEFAULT_PROJECT_NAME);

        project_refresh_list();
        return true;
    }
    return false;
}

Project* project_get_list(int* out_count)
{
    // Lazily populate the list ONLY if it hasn't been loaded yet
    if (g_projects == NULL || g_project_count == 0) 
    {
        project_refresh_list();
    }
    
    *out_count = g_project_count;
    return g_projects;
}