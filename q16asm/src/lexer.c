#include "lexer.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "assembler.h"

size_t tokenize(const char *src, struct Token *out) {
    const char *p = src;
    size_t n = 0;

    while (*p) {

        if (*p == ' ' || *p == '\t') { p++; continue; }


        if (*p == ';')
        {
            while (*p && *p != '\n') p++;
            continue;
        }

        if (*p == ',')
        { 
            out[n++] = (struct Token){ TK_COMMA, p, 1, 0 }; 
            p++; 
            continue; 
        }
        if (*p == ':')
        { 
            out[n++]= (struct Token){ TK_COLON, p, 1, 0 }; 
            p++;
            continue;
        }
        if (*p == '[')
        {
            out[n++] = (struct Token){TK_LBRACKET, p, 1, 0};
            p++;
            continue;
        }
        if (*p == ']')
        {
            out[n++] = (struct Token){TK_RBRACKET, p, 1, 0};
            p++;
            continue;
        }
        if (*p == '.')
        {
            out[n++] = (struct Token){TK_DOT, p, 1, 0};
            p++;
            continue;
        }
        if (*p == '%')
        {
            out[n++] = (struct Token){TK_PERCENT, p, 1, 0};
            p++;
            continue;
        }
        if (*p == '\n')
        {
            out[n++] = (struct Token){TK_EOL, p, 1, 0};
            p++;
            continue;
        }
        

        if (*p >= '0' && *p <= '9')
        {
            const char *start = p;
            char num[17] = {0};
            uint16_t value = 0;
            size_t i = 0;
            char *endptr;

            if (p[0] == '0' && p[1] == 'x')
            {
                p += 2;
                while ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'F')  || (*p >= 'a' && *p <= 'f'))
                {
                    num[i++] = *p++;
                }
                value = (uint16_t)strtoul(num, &endptr, 16);  
            }
            else if (p[0] == '0' && p[1] == 'b')
            {
                p += 2;
                while (*p == '1' || *p == '0')
                {
                    num[i++] = *p++;
                }
                value = (uint16_t)strtoul(num, &endptr, 2);  
            }
            else
            {
                while (*p >= '0' && *p <= '9')
                {
                    num[i++] = *p++;
                }
                value = (uint16_t)strtoul(num, &endptr, 10);
            }
            out[n++] = (struct Token){ TK_NUMBER, start, p - start, value };
            continue;
        }

        if ((*p >= 'a' && *p <= 'z') ||
            (*p >= 'A' && *p <= 'Z') ||
             *p == '_') {

            const char *start = p;
            while ((*p >= 'a' && *p <= 'z') ||
                   (*p >= 'A' && *p <= 'Z') ||
                   (*p >= '0' && *p <= '9') ||
                    *p == '_') {
                p++;
            }
            out[n++] = (struct Token){ TK_IDENT, start, p - start, 0 };
            continue;
        }
        fprintf(stderr, "Unexpected character: '%c' (0x%02x)\n", *p, *p);
        return 0;
    }

    out[n++] = (struct Token){ TK_EOF, p, 0, 0 };
    return n;
}
