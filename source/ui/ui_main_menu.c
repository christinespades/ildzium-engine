#include "ui_main_menu.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include "ui/ui.h"
#include <GLFW/glfw3.h>

extern GLFWwindow* g_window;

// Callback used by libcurl to write downloaded data into a string
static size_t write_to_string(void *ptr, size_t size, size_t nmemb, void *userp) {
    size_t total = size * nmemb;
    char **str = (char **)userp;
    size_t old_len = *str ? strlen(*str) : 0;

    *str = realloc(*str, old_len + total + 1);
    memcpy(*str + old_len, ptr, total);
    (*str)[old_len + total] = '\0';

    return total;
}

// Fetch a URL into a malloc'd string (caller must free)
char* fetch_readme(const char* url) {
    CURL *curl = curl_easy_init();
    char *result = NULL;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "ildzium-engine"); // GitHub requires User-Agent
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return result;
}

void setup_main_menu_controls(UI_Context* ctx) {
    char* readme = fetch_readme(
        "https://raw.githubusercontent.com/christinespades/ildzium-engine/main/README.md"
    );
    if (!readme) return;

    // Get window size
    int screen_w, screen_h;
    glfwGetWindowSize(g_window, &screen_w, &screen_h);

    int padding = 40; // leave margin around edges
    int x = padding;
    int y = padding;
    int w = screen_w - padding * 2;
    int h = screen_h - padding * 2;

    // Prepare wrapped text buffer
    static char wrapped_text[65536];
    wrapped_text[0] = '\0';

    int max_chars_per_line = (w / 6); // approximate: 6 px per char at smaller scale
    int line_len = 0;

    char* word = strtok(readme, " \n"); // split on space or newline
    while (word) {
        int word_len = (int)strlen(word);

        // If adding this word exceeds line width, add newline first
        if (line_len + word_len + 1 > max_chars_per_line) {
            strcat(wrapped_text, "\n");
            line_len = 0;
        }

        strcat(wrapped_text, word);
        strcat(wrapped_text, " ");
        line_len += word_len + 1;

        word = strtok(NULL, " \n");
    }

    // Add a single button covering most of the screen
    ui_add_button(ctx, x, y, w, h, wrapped_text, NULL, NULL, NULL);

    free(readme);
}