#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>
#include <stddef.h>
#include "compiler.h"


enum TokenKind {
    TK_EOF,
    TK_IDENT,
    TK_NUMBER,
    TK_STRING,
    TK_COMMA,
    TK_COLON,
    TK_SEMICOLON,
    TK_LBRACKET,
    TK_RBRACKET,
    TK_LBRACE,
    TK_RBRACE,
    TK_DOT,
    TK_PERCENT,
    TK_PLUS,
    TK_PLUSPLUS,
    TK_MINUS,
    TK_MINUSMINUS,
    TK_TILDA,
    TK_STAR,
    TK_SLASH,
    TK_GREATER,
    TK_LESS,
    TK_GREATEREQ,
    TK_LESSEQ,
    TK_EQ,
    TK_EQEQ,
    TK_BANGEQ,
    TK_AND,
    TK_ANDAND,
    TK_PIPE,
    TK_PIPEPIPE,
    TK_HASH,
    TK_BANG,
    TK_LPAREN,
    TK_RPAREN,
    TK_QUOTE,
    TK_DQUOTE,
    TK_INT,
    TK_CHAR,
    TK_VOID,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_FOR,
    TK_BREAK,
    TK_CONTINUE,
    TK_RETURN,
    TK_STRUCT,
    TK_SIZEOF
};

struct TokenStr {
    const char *ptr;
    uint8_t len;
};


struct Token {
    enum TokenKind kind;
    const char *start;
    size_t len;
    union {
        uint16_t u16;
        uint8_t u8;
        struct TokenStr str;
    } value;
};


struct Keyword {
    const char name[12];
    enum TokenKind kind;
};

size_t tokenize(const char *src, struct Token *out, struct Context *ctx);

#endif
