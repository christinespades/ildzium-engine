#include "ui_main_menu.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include "ui/ui.h"
#include "ui/ui_elements.h"
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
	char* readme = fetch_readme("https://raw.githubusercontent.com/christinespades/ildzium-engine/main/README.md");
    if (!readme) return;

    int screen_w, screen_h;
    glfwGetWindowSize(g_window, &screen_w, &screen_h);
    int padding = 40;
    int x = padding;
    int y = padding;
    int w = screen_w - padding * 2;
    int h = screen_h - padding * 2;

    // === Markdown processor with header levels ===
    static char processed[131072];
    processed[0] = '\0';
    char* dst = processed;
    const char* src = readme;
    int header_level = 0;

    while (*src) {
        // Detect headers at start of line
        if ((*src == '#' ) && (src == readme || *(src-1) == '\n' || *(src-1) == '\r')) {
            *dst++ = '\n';
            *dst++ = '\n';
            header_level = 0;
            while (*src == '#') {
                header_level++;
                src++;
            }
            while (*src == ' ' || *src == '\t') src++;

            // Store header level as a control character (we'll detect it later)
            if (header_level >= 1 && header_level <= 6) {
                *dst++ = (char)(0x01 + header_level);  // 0x02 = H1, 0x03 = H2, etc.
            }
            continue;
        }

        // Unordered lists
        else if ((*src == '-' || *src == '*' || *src == '+') &&
                 (src == readme || *(src-1) == '\n' || *(src-1) == '\r')) {
            *dst++ = '\n';
            *dst++ = ' ';
            strcpy(dst, "•");
			dst += strlen("•");
            *dst++ = ' ';
            src++;
            while (*src == ' ' || *src == '\t') src++;
            continue;
        }

        // Blockquotes
        else if (*src == '>' && (src == readme || *(src-1) == '\n' || *(src-1) == '\r')) {
            *dst++ = '\n';
            *dst++ = '\n';
            strcpy(dst, "“");
			dst += strlen("“");
            src++;
            while (*src == ' ' || *src == '\t') src++;
            continue;
        }

        // Code blocks (simple detection)
        else if (src[0] == '`' && src[1] == '`' && src[2] == '`') {
            *dst++ = '\n';
            *dst++ = '\n';
            *dst++ = '[';
            *dst++ = 'C';
            *dst++ = 'O';
            *dst++ = 'D';
            *dst++ = 'E';
            *dst++ = ']';
            *dst++ = '\n';
            src += 3;
            while (*src && !(src[0] == '`' && src[1] == '`' && src[2] == '`')) {
                *dst++ = (*src == '\n' ? '\n' : *src);
                src++;
            }
            if (*src) src += 3;
            *dst++ = '\n';
            continue;
        }

        // Normal newlines → paragraphs
        else if (*src == '\n' || *src == '\r') {
            src++;
            while (*src == '\n' || *src == '\r') src++;
            if (dst > processed && *(dst-1) != '\n')
                *dst++ = '\n';
            *dst++ = '\n';
            continue;
        }

        else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';

    // === Wrapping (preserves \n and control chars) ===
    static char wrapped_text[131072];
    wrapped_text[0] = '\0';

    int max_width_px = w - 30;
    int max_chars_per_line = max_width_px / 8;

    int line_len = 0;
    char* word = strtok(processed, " \t");

    while (word) {
        int word_len = (int)strlen(word);
        if (line_len + word_len + 1 > max_chars_per_line && line_len > 0) {
            strcat(wrapped_text, "\n");
            line_len = 0;
        }
        strcat(wrapped_text, word);
        strcat(wrapped_text, " ");
        line_len += word_len + 1;
        word = strtok(NULL, " \t");
    }

    ui_add_scrollable_text(ctx, x, y, w, h, wrapped_text);
    free(readme);
}