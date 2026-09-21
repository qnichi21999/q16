#ifndef PARSER_H
#define PARSER_H
#include "stdint.h"
#include "lexer.h"
#include "arena.h"

struct Parser {
    struct Token *tokens;
    size_t count;
    size_t current;
    struct Arena *arena;
};

enum AstKind {
    AST_PROGRAM,
    AST_FUNCTION,
    AST_PARAMETER,
    AST_BLOCK_STMT,
    AST_RETURN_STMT,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_FOR_STMT,
    AST_EXPR_STMT,
    AST_DECL_STMT,
    AST_INTEGER_LITERAL,
    AST_STRING_LITERAL,
    AST_IDENTIFIER,
    AST_UNARY_EXPR,
    AST_BINARY_EXPR,
    AST_CALL_EXPR,
    AST_INDEX_EXPR
    // AST_MEMBER_EXPR for structs
};

enum TypeKind {
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_INT,
    TYPE_POINTER,
};

struct Type {
    enum TypeKind kind;

    union {
        struct {
            enum TypeKind base;
        } pointer;
    };
};

struct AstNode;

struct AstNodeList {
    struct AstNode **items;
    size_t count;
    size_t capacity;
};

struct Precedence {
    enum TokenKind operator;
    short precedence;
};

struct AstNode {
    enum AstKind kind;

    union {
        struct {
            struct AstNodeList nodes;
        } program;

        struct {
            uint16_t value;
        } integer;

        struct {
            const char *ptr;
        } string;

        struct {
            struct AstNode *expression;
        } expr_stmt;

        struct {
            struct AstNode *name;
            struct Type type;
        } parameter;

        struct {
            struct AstNode *rhs;
            enum TokenKind op;
        } unary_expr;

        struct {
            struct AstNode *lhs;
            struct AstNode *rhs;
            enum TokenKind op;
        } binary_expr;

        struct {
            struct Token *name;
            struct Type return_type;
            struct AstNodeList params;
            struct AstNode *body;
        } function_decl_stmt;

        struct {
            struct Type type;
            struct AstNode *init_value;
        } decl_stmt;

        struct {
            struct AstNode *return_value;
        } return_stmt;

        struct {
            struct AstNode *condition;
            struct AstNode *then_branch;
            struct AstNode *else_branch;
        } if_stmt;

        struct {
            struct AstNodeList nodes;
        } block_stmt;

        struct {
            struct AstNode *name;
            struct AstNodeList args;
        } call_expr;

    };
};

struct AstNode *parse(struct Parser *p);

#endif