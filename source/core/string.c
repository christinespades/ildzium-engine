#include "pch.h"
#include "core/string.h"

int is_word_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

void wrap_text(const char* src, char* dst, size_t cap, int max_chars)
{
    size_t dst_len = 0;
    int line_len = 0;

    const char* p = src;

    while (*p && dst_len < cap - 1)
    {
        // Preserve explicit newlines
        if (*p == '\n')
        {
            dst[dst_len++] = '\n';
            line_len = 0;
            p++;
            continue;
        }

        // Skip spaces but preserve single spacing
        while (*p == ' ' || *p == '\t') p++;

        const char* word_start = p;

        while (*p && *p != ' ' && *p != '\t' && *p != '\n')
            p++;

        int word_len = (int)(p - word_start);
        if (word_len == 0) continue;

        // Wrap
        if (line_len + word_len + 1 > max_chars && line_len > 0)
        {
            dst[dst_len++] = '\n';
            line_len = 0;
        }

        // Copy word
        if (dst_len + word_len + 1 >= cap) break;

        memcpy(dst + dst_len, word_start, word_len);
        dst_len += word_len;

        dst[dst_len++] = ' ';
        line_len += word_len + 1;
    }

    dst[dst_len] = '\0';
}

// Callback used by libcurl to write downloaded data into a string
size_t write_to_string(void *ptr, size_t size, size_t nmemb, void *userp) {
    size_t total = size * nmemb;
    char **str = (char **)userp;
    size_t old_len = *str ? strlen(*str) : 0;

    *str = realloc(*str, old_len + total + 1);
    memcpy(*str + old_len, ptr, total);
    (*str)[old_len + total] = '\0';

    return total;
}