#pragma once

#define MD_BOLD_START   0x10
#define MD_BOLD_END     0x11
#define MD_ITALIC_START 0x12
#define MD_ITALIC_END   0x13
#define MD_LINK_START   0x14
#define MD_LINK_END     0x15

typedef enum {
    MD_PARAGRAPH,
    MD_HEADER,
    MD_LIST_ITEM,
    MD_CODE_BLOCK,
    MD_BLOCKQUOTE
} md_type;

typedef struct {
    md_type type;
    int level;         // for headers (1–6)
    const char* text;  // pointer into original buffer
    int length;
} md_block;

void markdown_to_text(const char* readme, char* processed, size_t cap);