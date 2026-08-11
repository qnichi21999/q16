#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>
#include <stddef.h>


enum TokenKind {
    TK_EOF,
    TK_IDENT,
    TK_NUMBER,
    TK_COMMA,
    TK_COLON,
    TK_LBRACKET,
    TK_RBRACKET,
    TK_DOT,
    TK_PERCENT,
    TK_PLUS,
    TK_MINUS,
    TK_STAR,
    TK_SLASH,
    TK_LPAREN,
    TK_RPAREN,
    TK_QUOTE,
    TK_DQUOTE,
    TK_EOL
};

struct Token {
    enum TokenKind kind;
    const char *start;
    size_t len;
    uint16_t value;
};

size_t tokenize(const char *src, struct Token *out);

#endif
