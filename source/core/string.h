#pragma once

int is_word_char(char c);
void wrap_text(const char* src, char* dst, size_t cap, int max_chars);
size_t write_to_string(void *ptr, size_t size, size_t nmemb, void *userp);