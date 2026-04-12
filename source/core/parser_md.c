#include "pch.h"
#include "core/parser_md.h"

void markdown_to_text(const char* readme, char* processed, size_t cap)
{
    char* dst = processed;
    const char* src = readme;

    int header_level = 0;

    while (*src && (size_t)(dst - processed) < cap - 1)
    {
        if (src[0] == '*' && src[1] == '*')
        {
            *dst++ = MD_BOLD_START;
            src += 2;

            while (*src && !(src[0] == '*' && src[1] == '*'))
                *dst++ = *src++;

            if (*src) {
                src += 2;
                *dst++ = MD_BOLD_END;
            }
            continue;
        }

        if (*src == '*')
        {
            *dst++ = MD_ITALIC_START;
            src++;

            while (*src && *src != '*')
                *dst++ = *src++;

            if (*src) {
                src++;
                *dst++ = MD_ITALIC_END;
            }
            continue;
        }

        if (*src == '[')
        {
            src++;
            *dst++ = MD_LINK_START;

            // text
            while (*src && *src != ']')
                *dst++ = *src++;

            *dst++ = '\0'; // separator between text and URL

            if (*src == ']') src++;
            if (*src == '(') src++;

            // url
            while (*src && *src != ')')
                *dst++ = *src++;

            if (*src) src++;

            *dst++ = MD_LINK_END;
            continue;
        }
        
        if ((*src == '#') && (src == readme || src[-1] == '\n' || src[-1] == '\r'))
        {
            if (dst > processed && dst[-1] != '\n') *dst++ = '\n';
            *dst++ = '\n';

            header_level = 0;
            while (*src == '#') { header_level++; src++; }
            while (*src == ' ' || *src == '\t') src++;

            if (header_level >= 1 && header_level <= 6)
                *dst++ = (char)(0x01 + header_level);

            continue;
        }

        if ((*src == '-' || *src == '*' || *src == '+') &&
            (src == readme || src[-1] == '\n' || src[-1] == '\r'))
        {
            *dst++ = '\n';
            *dst++ = '*';
            *dst++ = ' ';
            src++;
            while (*src == ' ' || *src == '\t') src++;
            continue;
        }

        if (*src == '`' && src[1] == '`' && src[2] == '`')
        {
            *dst++ = '\n';
            strcpy(dst, "[CODE]\n");
            dst += 7;

            src += 3;
            while (*src && !(src[0]=='`' && src[1]=='`' && src[2]=='`'))
                *dst++ = *src++;

            if (*src) src += 3;
            *dst++ = '\n';
            continue;
        }

        if (*src == '\n' || *src == '\r')
        {
            src++;
            while (*src == '\n' || *src == '\r') src++;
            if (dst > processed && dst[-1] != '\n') *dst++ = '\n';
            *dst++ = '\n';
            continue;
        }

        *dst++ = *src++;
    }

    *dst = '\0';
}