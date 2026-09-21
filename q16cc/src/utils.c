#include "stddef.h"
#include "lexer.h"
#include "parser.h"
#include "irgen.h"
#include "codegen.h"
#include "stdio.h"

const char *token_kind_name(enum TokenKind kind)
{
    switch (kind) {
        case TK_EOF:        return "EOF";
        case TK_IDENT:      return "IDENT";
        case TK_NUMBER:     return "NUMBER";
        case TK_COMMA:      return "COMMA";
        case TK_COLON:      return "COLON";
        case TK_SEMICOLON:  return "SEMICOLON";
        case TK_LBRACKET:   return "LBRACKET";
        case TK_RBRACKET:   return "RBRACKET";
        case TK_LBRACE:     return "LBRACE";
        case TK_RBRACE:     return "RBRACE";
        case TK_DOT:        return "DOT";
        case TK_PERCENT:    return "PERCENT";
        case TK_PLUS:       return "PLUS";
        case TK_PLUSPLUS:   return "PLUSPLUS";
        case TK_MINUS:      return "MINUS";
        case TK_MINUSMINUS: return "MINUSMINUS";
        case TK_STAR:       return "STAR";
        case TK_SLASH:      return "SLASH";
        case TK_GREATER:    return "GREATER";
        case TK_LESS:       return "LESS";
        case TK_GREATEREQ:  return "GREATEREQ";
        case TK_LESSEQ:     return "LESSEQ";
        case TK_EQ:         return "EQ";
        case TK_EQEQ:       return "EQEQ";
        case TK_AND:        return "AND";
        case TK_ANDAND:     return "ANDAND";
        case TK_PIPE:       return "PIPE";
        case TK_PIPEPIPE:   return "PIPEPIPE";
        case TK_HASH:       return "HASH";
        case TK_BANG:       return "BANG";
        case TK_LPAREN:     return "LPAREN";
        case TK_RPAREN:     return "RPAREN";
        case TK_QUOTE:      return "QUOTE";
        case TK_DQUOTE:     return "DQUOTE";
        case TK_INT:        return "INT";
        case TK_CHAR:       return "CHAR";
        case TK_VOID:       return "VOID";
        case TK_IF:         return "IF";
        case TK_ELSE:       return "ELSE";
        case TK_WHILE:      return "WHILE";
        case TK_FOR:        return "FOR";
        case TK_BREAK:      return "BREAK";
        case TK_CONTINUE:   return "CONTINUE";
        case TK_RETURN:     return "RETURN";
        case TK_STRUCT:     return "STRUCT";
        case TK_SIZEOF:     return "SIZEOF";
        case TK_STRING:     return "STRING";
        default:            return "UNKNOWN";
    }
}


void print_ir(struct IRNode *node, size_t tab_count)
{
    for (size_t i = 0; i < tab_count; ++i)
        printf("    ");
    switch (node->kind)
    {
        case IR_PROGRAM: {
            printf("Program\n");

            for (size_t i = 0; i < node->program.function->function.instructions.count; ++i)
                print_ir(node->program.function->function.instructions.items[i], tab_count + 1);

            break;
        }
        case IR_INSTRUCTION: {
            if (node->instruction.kind == I_UNARY)
            {
                printf("Unary\n");
            }
            else if (node->instruction.kind == I_RETURN)
            {
                printf("Return \"%s\"\n", node->instruction.i_return.return_value.value.identifier);
            }
            break;
        }
    }
}

void print_operand(struct Operand *operand)
{
    if (operand->kind == OP_IMM)
    {
        printf("%#x", operand->imm);
    }
    else if (operand->kind == OP_PSEUDO)
    {
        printf(operand->pseudo);
    }
    else if (operand->kind == OP_REGISTER)
    {
        printf("R%d", operand->reg);
    }
    else if (operand->kind == OP_STACK)
    {
        printf("STACK(%d)", operand->stack_offset);
    }

}

void print_asm_ir(struct AsmNode *node, size_t tab_count)
{
    for (size_t i = 0; i < tab_count; ++i)
        printf("    ");
    switch (node->kind)
    {
        case ASM_PROGRAM: {
            printf("Program\n");

            for (size_t i = 0; i < node->program.function->function.instructions->count; ++i)
                print_asm_ir(node->program.function->function.instructions->items[i], tab_count + 1);

            break;
        }
        case ASM_INSTRUCTION: {
            if (node->instruction.kind == ASM_I_ALLOCATE_STACK)
            {
                printf("ALLOCATE_STACK %d\n", node->instruction.allocate_stack.size);
            }
            else if (node->instruction.kind == ASM_I_MOV)
            {
                printf("MOV ");
                print_operand(node->instruction.mov.src);
                printf(" ");
                print_operand(node->instruction.mov.dst);
                printf("\n");

            }
            else if (node->instruction.kind == ASM_I_LOAD)
            {
                printf("LOAD ");
                print_operand(node->instruction.load.reg);
                printf(" ");
                print_operand(node->instruction.load.address);
                printf(" ");
                printf("%#x", node->instruction.load.offset);
                printf("\n");
            }
            else if (node->instruction.kind == ASM_I_STORE)
            {
                printf("STORE ");
                print_operand(node->instruction.store.reg);
                printf(" ");
                print_operand(node->instruction.store.address);
                printf(" ");
                printf("%#x", node->instruction.store.offset);
                printf("\n");
            }
            else if (node->instruction.kind == ASM_I_SET)
            {
                printf("SET");
                print_operand(node->instruction.set.reg);
                printf(" ");
                print_operand(node->instruction.set.imm);
                printf("\n");
            }
            else if (node->instruction.kind == ASM_I_UNARY)
            {
                printf("UNARY ");
                printf(token_kind_name(node->instruction.unary.operator));
                printf(" ");
                print_operand(node->instruction.unary.operand);
                printf("\n");
            }
            else if (node->instruction.kind == ASM_I_RET)
            {
                printf("RET\n");
            }
            break;
        }
    }
}

void print_ast(struct AstNode *node, size_t tab_count)
{
    for (size_t i = 0; i < tab_count; ++i)
        printf("    ");

    switch (node->kind)
    {
        case AST_PROGRAM: {
            printf("Program:\n");

            for (size_t i = 0; i < node->program.nodes.count; ++i)
                print_ast(node->program.nodes.items[i], tab_count + 1);

            break;
        }

        case AST_FUNCTION: {
            printf("Function:\n");

            for (size_t i = 0; i < tab_count + 1; ++i)
                printf("    ");

            printf("Name(\"%.*s\")\n", node->function_decl_stmt.name->len, node->function_decl_stmt.name->start);

            for (size_t i = 0; i < tab_count + 1; ++i)
                printf("    ");

            printf("ReturnType(%d)\n",
                   node->function_decl_stmt.return_type.kind);

            for (size_t i = 0;
                 i < node->function_decl_stmt.params.count;
                 ++i)
            {
                print_ast(
                    node->function_decl_stmt.params.items[i],
                    tab_count + 1
                );
            }

            print_ast(node->function_decl_stmt.body, tab_count + 1);

            break;
        }

        case AST_PARAMETER: {
            printf("Parameter:\n");

            print_ast(node->parameter.name, tab_count + 1);

            for (size_t i = 0; i < tab_count + 1; ++i)
                printf("    ");

            printf("Type(%d)\n", node->parameter.type.kind);

            break;
        }

        case AST_BLOCK_STMT: {
            printf("Block:\n");

            for (size_t i = 0;
                 i < node->block_stmt.nodes.count;
                 ++i)
            {
                print_ast(
                    node->block_stmt.nodes.items[i],
                    tab_count + 1
                );
            }

            break;
        }

        case AST_RETURN_STMT: {
            printf("Return:\n");

            if (node->return_stmt.return_value)
                print_ast(node->return_stmt.return_value, tab_count + 1);

            break;
        }

        case AST_IF_STMT: {
            printf("If:\n");

            for (size_t i = 0; i < tab_count + 1; ++i)
                printf("    ");

            printf("Condition:\n");
            print_ast(node->if_stmt.condition, tab_count + 2);

            for (size_t i = 0; i < tab_count + 1; ++i)
                printf("    ");

            printf("Then:\n");
            print_ast(node->if_stmt.then_branch, tab_count + 2);

            if (node->if_stmt.else_branch)
            {
                for (size_t i = 0; i < tab_count + 1; ++i)
                    printf("    ");

                printf("Else:\n");
                print_ast(node->if_stmt.else_branch, tab_count + 2);
            }

            break;
        }


        case AST_FOR_STMT: {
            printf("For:\n");

            break;
        }

        case AST_EXPR_STMT: {
            printf("ExpressionStmt:\n");

            print_ast(node->expr_stmt.expression, tab_count + 1);

            break;
        }

        case AST_DECL_STMT: {
            printf("Declaration:\n");

            for (size_t i = 0; i < tab_count + 1; ++i)
                printf("    ");

            printf("Type(%d)\n", node->decl_stmt.type.kind);

            if (node->decl_stmt.init_value)
                print_ast(node->decl_stmt.init_value, tab_count + 1);

            break;
        }

        case AST_INTEGER_LITERAL: {
            printf("Integer(%u)\n", node->integer.value);
            break;
        }

        case AST_STRING_LITERAL: {
            printf("String(\"%s\")\n", node->string.ptr);
            break;
        }

        case AST_IDENTIFIER: {
            printf("Identifier(\"%s\")\n", "fuck i dont have it");
            break;
        }

        case AST_UNARY_EXPR: {
            printf("Unary:\n");

            for (size_t i = 0; i < tab_count + 1; ++i)
                printf("    ");

            printf("Operator(%d)\n", node->unary_expr.op);

            print_ast(node->unary_expr.rhs, tab_count + 1);

            break;
        }

        case AST_BINARY_EXPR: {
            printf("Binary:\n");

            print_ast(node->binary_expr.lhs, tab_count + 1);

            for (size_t i = 0; i < tab_count + 1; ++i)
                printf("    ");

            printf("Operator(%d)\n", node->binary_expr.op);

            print_ast(node->binary_expr.rhs, tab_count + 1);

            break;
        }

        default: {
            printf("Unknown");
            break;
        }
    }
}

void print_tokens(struct Token *tokens, size_t token_count)
{
    for (size_t i = 0; i < token_count; ++i) 
    {
        if (tokens[i].kind == TK_STRING)
        {
            printf("[%zu] %-10s %.*s %.*s \n",
                i,
                token_kind_name(tokens[i].kind),
                tokens[i].len,
                tokens[i].start,
                tokens[i].value.str.len,
                tokens[i].value.str.ptr);
        }
        else
        {
            printf("[%zu] %-10s \"%.*s\"\n",
                i,
                token_kind_name(tokens[i].kind),
                tokens[i].len,
                tokens[i].start);
        }
    }
}