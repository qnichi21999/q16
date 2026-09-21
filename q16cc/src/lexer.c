#include "lexer.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "arena.h"
#include "compiler.h"
#define N_KEYWORDS 12
#define MAX_STRING_LENGTH 256

const struct Keyword keywords[] = {
    { "int",      TK_INT      },
    { "char",     TK_CHAR     },
    { "void",     TK_VOID     },
    { "if",       TK_IF       },
    { "else",     TK_ELSE     },
    { "while",    TK_WHILE    },
    { "for",      TK_FOR      },
    { "break",    TK_BREAK    },
    { "continue", TK_CONTINUE },
    { "return",   TK_RETURN   },
    { "struct",   TK_STRUCT   },
    { "sizeof",   TK_SIZEOF   }
};

size_t tokenize(const char *src, struct Token *out, struct Context *ctx) {
    const char *p = src;
    size_t n = 0;

    while (*p) {

        if (*p == ' ' || *p == '\t' || *p == '\n') { p++; continue; }


        if (*p == ';')
        {
            out[n++] = (struct Token){ TK_SEMICOLON, p, 1, 0 }; 
            p++; 
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
        if (*p == '(')
        {
            out[n++] = (struct Token){TK_LPAREN, p, 1, 0};
            p++;
            continue;
        }
        if (*p == ')')
        {
            out[n++] = (struct Token){TK_RPAREN, p, 1, 0};
            p++;
            continue;
        }
        if (*p == '{')
        {
            out[n++] = (struct Token){TK_LBRACE, p, 1, 0};
            p++;
            continue;
        }
        if (*p == '}')
        {
            out[n++] = (struct Token){TK_RBRACE, p, 1, 0};
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
        if (*p == '+')
        {
            p++;
            if (*p == '+')
            {
                out[n++] = (struct Token){TK_PLUSPLUS, p-1, 2, 0};
                p++;
                continue;
            }
            out[n++] = (struct Token){TK_PLUS, p-1, 1, 0};
            continue;
        }
        if (*p == '-')
        {
            p++;
            if (*p == '-')
            {
                out[n++] = (struct Token){TK_MINUSMINUS, p-1, 2, 0};
                p++;
                continue;
            }
            out[n++] = (struct Token){TK_MINUS, p-1, 1, 0};
            continue;
        }
        if (*p == '*')
        {
            out[n++] = (struct Token){TK_STAR, p, 1, 0};
            p++;
            continue;
        }
        if (*p == '#')
        {
            out[n++] = (struct Token){TK_HASH, p, 1, 0};
            p++;
            continue;
        }
        if (*p == '~')
        {
            out[n++] = (struct Token){TK_HASH, p, 1, 0};
            p++;
            continue;
        }
        if (*p == '!')
        {
            p++;
            if (*p == '=')
            {
                out[n++] = (struct Token){TK_BANGEQ, p-1, 2, 0};
                continue;
            }
            out[n++] = (struct Token){TK_BANG, p-1, 1, 0};
            continue;
        }
        if (*p == '/')
        {
            p++;
            if (*p == '/')
            {
                while (*p && *p != '\n') p++;
                continue;
            }
            else
            {
                out[n++] = (struct Token){TK_SLASH, p-1, 1, 0};
            }    
            continue;
        }
        if (*p == '=')
        {
            p++;
            if (*p == '=')
            {
                out[n++] = (struct Token){TK_EQEQ, p-1, 2, 0};
                continue;
            }
            out[n++] = (struct Token){TK_EQ, p-1, 1, 0};
            continue;
        }
        if (*p == '&')
        {
            p++;
            if (*p == '&')
            {
                out[n++] = (struct Token){TK_ANDAND, p-1, 2, 0};
                p++;
                continue;
            }
            out[n++] = (struct Token){TK_AND, p-1, 1, 0};
            continue;
        }
        if (*p == '|')
        {
            p++;
            if (*p == '|')
            {
                out[n++] = (struct Token){TK_PIPEPIPE, p-1, 2, 0};
                p++;
                continue;
            }
            out[n++] = (struct Token){TK_PIPE, p-1, 1, 0};
            continue;
        }

        if (*p == '>')
        {
            p++;
            if (*p == '=')
            {
                out[n++] = (struct Token){TK_GREATEREQ, p-1, 2, 0};
                p++;
                continue;
            }
            out[n++] = (struct Token){TK_GREATER, p-1, 1, 0};
            continue;
        }
        if (*p == '<')
        {
            p++;
            if (*p == '=')
            {
                out[n++] = (struct Token){TK_LESSEQ, p-1, 2, 0};
                p++;
                continue;
            }
            out[n++] = (struct Token){TK_LESS, p-1, 1, 0};
            continue;
        }
        if (*p == '\'')
        {
            p++;
            if (!(*p > 0 && *p < 256) && *p != '\'')
            {
                fprintf(stderr, "Failed to parse char (0x%02x)\n", *p);
                exit(1);
            }
            struct Token chr = {TK_NUMBER, p, 1, .value.u8 = (uint8_t)(*p)};
            p++;
            if (*p != '\'')
            {
                fprintf(stderr, "Failed to parse char: closing ' is needed (0x%02x)\n", *p);
                exit(1);
            }
            out[n++] = chr;
            p++;
            continue;
        }
        if (*p == '\"')
        {
            p++;
            char string[256] = {0};
            size_t i = 0;
            size_t n_escaped = 0;
            while (*p != '\"')
            {
                if (i > 255)
                {
                    fprintf(stderr, "Failed to parse string: string's length cannot be more than 255 (0x%02x)\n", *p);
                    exit(1);
                }
                if (*p == '\\')
                {
                    p++;
                    if (*p == 'n')
                    {
                        string[i] = '\n';
                    }
                    else if (*p == 't')
                    {
                        string[i] = '\t';
                    }
                    else if (*p == 'r')
                    {
                        string[i] = '\r';
                    }
                    else if (*p == 'a')
                    {
                        string[i] = '\a';
                    }
                    else if (*p == 'b')
                    {
                        string[i] = '\b';
                    }
                    else if (*p == '\'')
                    {
                        string[i] = '\'';
                    }
                    else if (*p == '\"')
                    {
                        string[i] = '\"';
                    }
                    else if (*p == 't')
                    {
                        string[i] = '\t';
                    }
                    else if (*p == '0')
                    {
                        string[i] = '\0';
                    }
                    else
                    {
                        string[i] = *p;
                    }
                    n_escaped++;
                }
                else
                {
                    string[i] = *p;
                }
                i++;
                p++;
            }
            size_t string_len = strlen(string);
            char *string_ptr = arena_alloc(&(ctx->rodata_arena), string_len+1);
            strcpy(string_ptr, string);
            out[n++] = (struct Token){
                TK_STRING,
                p - string_len - 1 - n_escaped,
                string_len + 2 + n_escaped,
                .value.str = (struct TokenStr){
                    string_ptr,
                    string_len
                }
            };
            p++;
            continue;
        }
        

        if (*p >= '0' && *p <= '9')
        {
            const char *start = p;
            char num[17] = {0};
            uint16_t val = 0;
            size_t i = 0;
            char *endptr;

            if (p[0] == '0' && p[1] == 'x')
            {
                p += 2;
                while ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'F')  || (*p >= 'a' && *p <= 'f'))
                {
                    num[i++] = *p++;
                }
                val = (uint16_t)strtoul(num, &endptr, 16);  
            }
            else if (p[0] == '0' && p[1] == 'b')
            {
                p += 2;
                while (*p == '1' || *p == '0')
                {
                    num[i++] = *p++;
                }
                val = (uint16_t)strtoul(num, &endptr, 2);  
            }
            else
            {
                while (*p >= '0' && *p <= '9')
                {
                    num[i++] = *p++;
                }
                val = (uint16_t)strtoul(num, &endptr, 10);
            }
            out[n++] = (struct Token){ TK_NUMBER, start, p - start, .value.u16 = val };
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
            const char ident[12] = {0};
            strncpy(ident, start, p-start);
            out[n++] = (struct Token){ TK_IDENT, start, p - start, 0 };
            for (int i = 0; i < N_KEYWORDS; ++i)
            {
                if (!strcmp(ident, keywords[i].name))
                {
                    out[n-1] = (struct Token){ keywords[i].kind, start, p - start, 0 };
                    break;
                }
            }
            continue;
        }
        fprintf(stderr, "Unexpected character: '%c' (0x%02x)\n", *p, *p);
        return 0;
    }

    out[n++] = (struct Token){ TK_EOF, p, 0, 0 };
    return n;
}
