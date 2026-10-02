#include "irgen.h"
#include "parser.h"
#include "string.h"
#include "stdio.h"
#include "utils.h"

const char *make_temp_name(struct IRGen *irgen)
{
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "tmp.%zu", irgen->temporary_id++);
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

char *make_label(struct IRGen *irgen)
{
    int len = snprintf(NULL, 0, "L%zu", irgen->label_id);
    char *label = arena_alloc(irgen->arena, len + 1);
    snprintf(label, len + 1, "L%zu", irgen->label_id);
    irgen->label_id++;
    return label;
}

struct IRNode *generate_label(struct IRGen *irgen, char *name)
{
    struct IRNode *label = arena_alloc(irgen->arena, sizeof(*label));
    *label = (struct IRNode){
        .kind = IR_INSTRUCTION,
        .instruction = {
            .kind = I_LABEL,
            .i_label = {
                .name = name
            }
        }
    };
    return label;
}

struct IRNode *generate_jz(struct IRGen *irgen, struct IRNode *src, struct IRNode *target)
{
    struct IRNode *jz = arena_alloc(irgen->arena, sizeof(*jz));
    *jz = (struct IRNode){
        .kind = IR_INSTRUCTION,
        .instruction = {
            .kind = I_JZ,
            .i_jz = {
                .src = src->val.value,
                .target = target
            }
        }
    };
    return jz;
}


struct IRNode *generate_jmp(struct IRGen *irgen, struct IRNode *target)
{
    struct IRNode *jmp = arena_alloc(irgen->arena, sizeof(*jmp));
    *jmp = (struct IRNode){
        .kind = IR_INSTRUCTION,
        .instruction = {
            .kind = I_JMP,
            .i_jmp = {
                .target = target
            }
        }
    };
    return jmp;
}

struct IRNode *generate_je(struct IRGen *irgen, struct IRNode *src, struct IRNode *dst, struct IRNode *target)
{
    struct IRNode *je = arena_alloc(irgen->arena, sizeof(*je));
    *je = (struct IRNode){
        .kind = IR_INSTRUCTION,
        .instruction = {
            .kind = I_JE,
            .i_je = {
                .src = src->val.value,
                .dst = dst->val.value,
                .target = target
            }
        }
    };
    return je;
}

struct IRNode *generate_jg(struct IRGen *irgen, struct IRNode *src, struct IRNode *dst, struct IRNode *target)
{
    struct IRNode *jg = arena_alloc(irgen->arena, sizeof(*jg));
    *jg = (struct IRNode){
        .kind = IR_INSTRUCTION,
        .instruction = {
            .kind = I_JG,
            .i_jg = {
                .src = src->val.value,
                .dst = dst->val.value,
                .target = target
            }
        }
    };
    return jg;
}


struct IRNode *generate_copy(struct IRGen *irgen, struct IRNode *src, struct IRNode *dst)
{
    struct IRNode *copy = arena_alloc(irgen->arena, sizeof(*copy));
    *copy = (struct IRNode){
        .kind = IR_INSTRUCTION,
        .instruction = {
            .kind = I_COPY,
            .i_copy = {
                .src = src->val.value,
                .dst = dst->val.value
            }
        }
    };
    return copy;
}

struct IRNode *generate_imm(struct IRGen *irgen, uint16_t imm)
{
    struct IRNode *imm_val = arena_alloc(irgen->arena, sizeof(*imm_val));
    *imm_val = (struct IRNode){
        .kind = IR_VAR,
        .val.value = {
            .kind = V_INT,
            .value.integer = imm
        }
    };
    return imm_val;
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
    else if (ast_node->kind == AST_BINARY_EXPR)
    {
        struct IRNode *src1 = emit_tac(irgen, ast_node->binary_expr.lhs, instructions);
        struct IRNode *src2 = emit_tac(irgen, ast_node->binary_expr.rhs, instructions);
        struct IRNode *dst = make_ir_temp(irgen);

        if (ast_node->binary_expr.op == TK_ANDAND || 
            ast_node->binary_expr.op == TK_PIPEPIPE ||
            ast_node->binary_expr.op == TK_EQEQ ||
            ast_node->binary_expr.op == TK_BANGEQ ||
            ast_node->binary_expr.op == TK_GREATER ||
            ast_node->binary_expr.op == TK_GREATEREQ ||
            ast_node->binary_expr.op == TK_LESS ||
            ast_node->binary_expr.op == TK_LESSEQ
        )
        {
            struct IRNode *result_label = generate_label(irgen, make_label(irgen));

            struct IRNode *jz1 = generate_jz(irgen, src1, result_label);
            struct IRNode *jz2 = generate_jz(irgen, src2, result_label);

            struct IRNode *je = generate_je(irgen, src1, src2, result_label);
            struct IRNode *jg;
            if (ast_node->binary_expr.op == TK_GREATER || ast_node->binary_expr.op == TK_GREATEREQ)
            {
                jg = generate_jg(irgen, src2, src1, result_label);
            }
            else if (ast_node->binary_expr.op == TK_LESS || ast_node->binary_expr.op == TK_LESSEQ)
            {
                jg = generate_jg(irgen, src1, src2, result_label);
            }

            struct IRNode *result1 = generate_copy(irgen, generate_imm(irgen, 1), dst);
            struct IRNode *result0 = generate_copy(irgen, generate_imm(irgen, 0), dst);
            struct IRNode *end_label = generate_label(irgen, make_label(irgen));
            struct IRNode *jmp_end = generate_jmp(irgen, end_label);
            if (ast_node->binary_expr.op == TK_ANDAND)
            {
                LIST_PUSH(irgen->arena, instructions, jz1, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, jz2, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result1, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, jmp_end, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result_label, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result0, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, end_label, struct IRNode);
            }
            else if (ast_node->binary_expr.op == TK_PIPEPIPE)
            {
                LIST_PUSH(irgen->arena, instructions, jz1, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, jz2, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result0, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, jmp_end, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result_label, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result1, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, end_label, struct IRNode);
            }
            else if (ast_node->binary_expr.op == TK_EQEQ)
            {
                LIST_PUSH(irgen->arena, instructions, je, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result0, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, jmp_end, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result_label, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result1, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, end_label, struct IRNode);
            }
            else if (ast_node->binary_expr.op == TK_BANGEQ)
            {
                LIST_PUSH(irgen->arena, instructions, je, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result1, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, jmp_end, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result_label, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result0, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, end_label, struct IRNode);
            }
            else if (ast_node->binary_expr.op == TK_GREATER || 
                     ast_node->binary_expr.op == TK_LESS ||
                     ast_node->binary_expr.op == TK_GREATEREQ ||
                     ast_node->binary_expr.op == TK_LESSEQ)
            {
                if (ast_node->binary_expr.op == TK_GREATEREQ || ast_node->binary_expr.op == TK_LESSEQ)
                {
                    LIST_PUSH(irgen->arena, instructions, je, struct IRNode);
                }
                LIST_PUSH(irgen->arena, instructions, jg, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result0, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, jmp_end, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result_label, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, result1, struct IRNode);
                LIST_PUSH(irgen->arena, instructions, end_label, struct IRNode);
            }
            return dst; 
        }

        struct IRNode *binary = arena_alloc(irgen->arena, sizeof(*binary));
        *binary = (struct IRNode) {
            .kind = IR_INSTRUCTION,
            .instruction = {
                .kind = I_BINARY,
                .i_binary = {
                    .src1 = src1->val.value,
                    .src2 = src2->val.value,
                    .dst = dst->val.value,
                    .op = ast_node->binary_expr.op
                }
            }
        };
        LIST_PUSH(irgen->arena, instructions, binary, struct IRNode);
        return dst;
    }
    return NULL;
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