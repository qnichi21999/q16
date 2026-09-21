#include "parser.h"
#include "stdbool.h"
#include "stdlib.h"
#include "stdio.h"
#include "utils.h"
#define N_OPERATORS 13


struct Precedence precedence_table[] = {
    {TK_STAR,      5},
    {TK_SLASH,     5},
    {TK_PERCENT,   5},
    {TK_PLUS,      4},
    {TK_MINUS,     4},
    {TK_GREATER,   3},
    {TK_LESS,      3},
    {TK_GREATEREQ, 3},
    {TK_LESSEQ,    3},
    {TK_EQEQ,      2},
    {TK_BANGEQ,    2},
    {TK_ANDAND,    1},
    {TK_PIPEPIPE,  0}
};

short find_precedence(enum TokenKind operator)
{
    for (int i = 0; i < N_OPERATORS; ++i)
    {
        if(operator == precedence_table[i].operator)
        {
            return precedence_table[i].precedence;
        }
    }
    return -1;
}

bool check(struct Token *token, enum TokenKind kind)
{
    return token->kind == kind;
}

struct Token *advance(struct Parser *p)
{
    p->tokens++;
    return p->tokens - 1;
}

struct Token *peek(struct Token *tokens)
{
    if (check(tokens, TK_EOF))
    {
        fprintf(stderr, "Syntax error");
        exit(1);
    }
    return tokens + 1;
}

bool match(struct Parser *p, enum TokenKind kind)
{
    if (check(p->tokens, kind))
    {
        advance(p);
        return true;
    }
    return false;
}

struct Token *consume(struct Parser *p, enum TokenKind kind, const char *err_msg)
{
    if (!check(p->tokens, kind))
    {
        fprintf(stderr, err_msg);
        exit(1);
    }
    return advance(p);
}


struct AstNode *parse_expr(struct Parser *p, short min_prec);
struct AstNode *parse_factor(struct Parser *p);
struct AstNode *parse_block_stmt(struct Parser *p);
struct AstNode *parse_statement(struct Parser *p);


struct AstNode *parse_factor(struct Parser *p)
{
    if (check(p->tokens, TK_NUMBER))
    {
        struct AstNode *integer = arena_alloc(p->arena, sizeof(*integer));
        integer->kind = AST_INTEGER_LITERAL;
        integer->integer.value = advance(p)->value.u16;
        return integer;
    }
    else if (check(p->tokens, TK_STRING))
    {
        struct AstNode *string = arena_alloc(p->arena, sizeof(*string));
        string->kind = AST_STRING_LITERAL;
        string->string.ptr = advance(p)->value.str.ptr;
        printf("%s\n", (p->tokens-1)->value.str.ptr);
        return string;
    }
    else if (check(p->tokens, TK_LPAREN))
    {
        advance(p);
        struct AstNode *inner_expr = parse_expr(p, 0);
        consume(p, TK_RPAREN, "Expected )\n");
        return inner_expr;
    }
    else if (check(p->tokens, TK_MINUS) || 
             check(p->tokens, TK_MINUSMINUS) || 
             check(p->tokens, TK_PLUSPLUS) ||
             check(p->tokens, TK_TILDA) ||
             check(p->tokens, TK_STAR) ||
             check(p->tokens, TK_AND))
    {
        enum TokenKind op = advance(p)->kind;
        struct AstNode *unary = arena_alloc(p->arena, sizeof(*unary));
        unary->kind = AST_UNARY_EXPR;
        unary->unary_expr.op = op;
        unary->unary_expr.rhs = parse_factor(p);
        return unary;
    }
    else
    {
        fprintf(stderr, "Malformed expression");
        exit(1);
    }
}
struct AstNode *parse_expr(struct Parser *p, short min_prec)
{
    struct AstNode *left = parse_factor(p);

    for (;;)
    {
        short prec = find_precedence(p->tokens->kind);

        if (prec < min_prec)
            break;

        enum TokenKind operator = advance(p)->kind;

        struct AstNode *right = parse_expr(p, prec + 1);

        struct AstNode *binary = arena_alloc(p->arena, sizeof(*binary));
        binary->kind = AST_BINARY_EXPR;
        binary->binary_expr.lhs = left;
        binary->binary_expr.op = operator;
        binary->binary_expr.rhs = right;

        left = binary;
    }

    return left;
}


struct AstNode *parse_parameter(struct Parser *p)
{
    struct Type type;
    struct Token *type_token = advance(p);
    if (check(p->tokens, TK_VOID))
    {
        type.kind = TYPE_VOID;
    }
    else if (check(p->tokens, TK_INT))
    {
        type.kind = TYPE_INT;
    }
    else if (check(p->tokens, TK_CHAR))
    {
        type.kind = TYPE_CHAR;
    }
    if (match(p, TK_STAR))
    {
        type.pointer.base = type.kind;
        type.kind = TYPE_POINTER;
    }
    struct Token *name = consume(p, TK_IDENT, "Expected an identifier in parameter declaration");
    struct AstNode *parameter = arena_alloc(p->arena, sizeof(*parameter));
    parameter->kind = AST_PARAMETER;
    parameter->parameter.name = name;
    parameter->parameter.type = type;
    return parameter;
}

void parse_parameters(struct Parser *p, struct AstNodeList *list)
{
    if (check(p->tokens, TK_VOID) && peek(p->tokens)->kind == TK_RPAREN)
    {
        advance(p);
        return;
    }
    LIST_PUSH(p->arena, list, parse_parameter(p), struct AstNode);
    while (match(p, TK_COMMA))
        LIST_PUSH(p->arena, list, parse_parameter(p), struct AstNode);
}

struct AstNode *parse_declaration_stmt(struct Parser *p)
{
    struct Type type;
    struct Token *type_token = advance(p);
    if (check(p->tokens, TK_VOID))
    {
        type.kind = TYPE_VOID;
    }
    else if (check(p->tokens, TK_NUMBER))
    {
        type.kind = TYPE_INT;
    }
    else if (check(p->tokens, TK_CHAR))
    {
        type.kind = TYPE_CHAR;
    }
    if (match(p, TK_STAR))
    {
        type.pointer.base = type.kind;
        type.kind = TYPE_POINTER;
    }
    struct Token *name = consume(p, TK_IDENT, "Expected an identifier in declaration\n");
    struct AstNode *decl_stmt = arena_alloc(p->arena, sizeof(*decl_stmt));

    decl_stmt->kind = AST_DECL_STMT;
    decl_stmt->decl_stmt.type = type;
    if (match(p, TK_EQ))
    {
        decl_stmt->decl_stmt.init_value = parse_expr(p, 0);
        consume(p, TK_SEMICOLON, "Expected ; after declaration");
    }
    else if (match(p, TK_LPAREN))
    {
        decl_stmt->kind = AST_FUNCTION;
        decl_stmt->function_decl_stmt.name = name;
        decl_stmt->function_decl_stmt.return_type = type;
        parse_parameters(p, &(decl_stmt->function_decl_stmt.params));
        consume(p, TK_RPAREN, "Expected a closing ) in function declaration");
        decl_stmt->function_decl_stmt.body = parse_statement(p);
    }
    else
    {
        consume(p, TK_SEMICOLON, "Expected ; after declaration");
    }
    return decl_stmt;
}


struct AstNode *parse_expression_stmt()
{

}

struct AstNode *parse_block_stmt(struct Parser *p)
{
    struct AstNode *block_stmt = arena_alloc(p->arena, sizeof(*block_stmt));
    block_stmt->kind = AST_BLOCK_STMT;
    while (!check(p->tokens, TK_RBRACE))
    {
        LIST_PUSH(p->arena,  &(block_stmt->block_stmt.nodes), parse_statement(p), struct AstNode);
    }
        
    consume(p, TK_RBRACE, "Expected a } at the end of a block");
    return block_stmt;
}

struct AstNode *parse_return_stmt(struct Parser *p)
{
    struct AstNode *return_stmt = arena_alloc(p->arena, sizeof(*return_stmt));
    return_stmt->kind = AST_RETURN_STMT;
    return_stmt->return_stmt.return_value = parse_expr(p, 0);
    consume(p, TK_SEMICOLON, "Expected a ; after return statement");
    return return_stmt;
}

struct AstNode *parse_statement(struct Parser *p)
{
    if (check(p->tokens, TK_VOID) || 
        check(p->tokens, TK_CHAR) ||
        check(p->tokens, TK_INT))
            return parse_declaration_stmt(p);
    else if (match(p, TK_LBRACE))
    {
        return parse_block_stmt(p);
    }
    else if (match(p, TK_RETURN))
    {
        return parse_return_stmt(p);
    }
    
    // else if (check())
}

struct AstNode *parse(struct Parser *p)
{
    struct AstNode *root = arena_alloc(p->arena, sizeof(*root));
    root->kind = AST_PROGRAM;
    while (p->tokens->kind != TK_EOF)
        LIST_PUSH(p->arena, &(root->program.nodes), parse_statement(p), struct AstNode);
    return root;
}