#include "irgen.h"
#include "parser.h"
#include "string.h"
#include "stdio.h"
#include "utils.h"

const char *make_temp_name(struct IRGen *irgen)
{
    char tmp[32];
    int len = snprintf(tmp, sizeof(tmp), "tmp.%zu", irgen->temporary_id++);
    char *result = arena_alloc(irgen->arena, 32);
    memcpy(result, tmp, 32);
    return result;
}

struct IRNode *make_ir_temp(struct IRGen *irgen)
{
    struct IRNode *node = arena_alloc(irgen->arena, sizeof(*node));

    *node = (struct IRNode) {
        .kind = IR_VAR,
        .val.value = {
            .kind = V_VAR,
            .value.identifier = make_temp_name(irgen),
        },
    };

    return node;
}

struct IRNode *emit_tac(struct IRGen *irgen, struct AstNode *ast_node, struct IRNodeList *instructions)
{
    if (ast_node->kind == AST_INTEGER_LITERAL)
    {
        struct IRNode *constant = arena_alloc(irgen->arena, sizeof(*constant));
        *constant = (struct IRNode) {
            .kind = IR_VAR,
            .val.value = {
                .kind = V_INT,
                .value.integer = ast_node->integer.value,
            },
        };
        return constant;
    }
    else if (ast_node->kind == AST_UNARY_EXPR)
    {
        struct IRNode *src = emit_tac(irgen, ast_node->unary_expr.rhs, instructions);
        struct IRNode *dst = make_ir_temp(irgen);

        struct IRNode *unary = arena_alloc(irgen->arena, sizeof(*unary));
        *unary = (struct IRNode) {
            .kind = IR_INSTRUCTION,
            .instruction = {
                .kind = I_UNARY,
                .i_unary = {
                    .dst = dst->val.value,
                    .src = src->val.value,
                    .op = ast_node->unary_expr.op
                    }
                }
        };
        LIST_PUSH(irgen->arena, instructions, unary, struct IRNode);
        return dst;
    }
}

void emit_return(struct IRGen *irgen, struct IRNode *dst, struct IRNodeList *instructions)
{
    struct IRNode *i_return = arena_alloc(irgen->arena, sizeof(*i_return));
    *i_return = (struct IRNode) {
        .kind = IR_INSTRUCTION,
        .instruction = {
            .kind = I_RETURN,
            .i_return = {
                .return_value = dst->val.value
            }
        }
    };
    LIST_PUSH(irgen->arena, instructions, i_return, struct IRNode);
}

struct IRNode *generate_ir(struct IRGen *irgen, struct AstNode *ast)
{
    struct IRNode *program = arena_alloc(irgen->arena, sizeof(*program));
    program->kind = IR_PROGRAM;
    if (ast->kind == AST_PROGRAM)
    {
        for (size_t i = 0; i < ast->program.nodes.count; ++i)
        {
            struct AstNode *node = ast->program.nodes.items[i];
            if (node->kind == AST_FUNCTION)
            {
                program->program.function = arena_alloc(irgen->arena, sizeof(*(program->program.function)));
                program->program.function->function.name = node->function_decl_stmt.name;
                struct AstNode *body = node->function_decl_stmt.body;
                if (body->kind == AST_BLOCK_STMT)
                {
                    struct AstNodeList *statements = &body->block_stmt.nodes;

                    for (size_t j = 0; j < statements->count; ++j)
                    {
                        if (statements->items[i]->kind == AST_RETURN_STMT)
                        {
                            struct IRNode *dst = emit_tac(irgen,
                                                         statements->items[i]->return_stmt.return_value,
                                                         &(program->program.function->function.instructions));
                            emit_return(irgen, dst, &(program->program.function->function.instructions));
                        }
                        else
                        {
                            emit_tac(irgen, statements->items[i], &(program->program.function->function.instructions));
                        }
                    }
                }
                else
                {
                    emit_tac(irgen, body, &(program->program.function->function.instructions));
                }
            }

        }
        
    }
    
    return program;
}