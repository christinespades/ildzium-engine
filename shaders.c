#include <stdio.h>    // FILE, fopen, fread, fseek, ftell, fclose, printf
#include <stdlib.h>   // malloc, free, exit, NULL
#include <stdint.h>   // uint32_t

uint32_t* load_spirv(const char* filename, size_t* out_size)
{
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open shader: %s\n", filename);
        *out_size = 0;
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size % 4 != 0) {
        printf("Invalid SPIR-V file size: %s\n", filename);
        fclose(file);
        *out_size = 0;
        return NULL;
    }

    uint32_t* code = malloc(file_size);
    fread(code, 1, file_size, file);
    fclose(file);

    *out_size = file_size;
    return code;
}