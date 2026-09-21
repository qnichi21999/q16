#include "codegen.h"
#include "irgen.h"
#include "utils.h"
#include "stdio.h"
#include "string.h"

struct Operand *ir_var_to_operand(struct CodeGen *c, struct Var var)
{
    struct Operand *operand = arena_alloc(c->arena, sizeof(*operand));
    if (var.kind == V_INT)
    {
        operand->kind = OP_IMM;
        operand->imm = var.value.integer;
    }
    else if (var.kind == V_VAR)
    {
        operand->kind = OP_PSEUDO;
        operand->pseudo = var.value.identifier;
    }
    return operand;
}

struct AsmNode *generate_asm_mov(struct CodeGen *c, struct Operand *src, struct Operand *dst)
{
    struct AsmNode *mov = arena_alloc(c->arena, sizeof(*mov));
    *mov = (struct AsmNode){
        .kind = ASM_INSTRUCTION,
        .instruction = {
            .kind = ASM_I_MOV,
            .mov = {
                .dst = dst,
                .src = src
            }
        }
    };
    return mov;
}

struct AsmNode *generate_asm_load(struct CodeGen *c, struct Operand *reg, struct Operand *address, uint16_t offset)
{
    struct AsmNode *load = arena_alloc(c->arena, sizeof(*load));                
    *load = (struct AsmNode){
        .kind = ASM_INSTRUCTION,
        .instruction = {
            .kind = ASM_I_LOAD,
            .load = {
                .reg = reg,
                .address = address,
                .offset = offset,
            }
        }
    };
    return load;
}

struct AsmNode *generate_asm_store(struct CodeGen *c, struct Operand *reg, struct Operand *address, uint16_t offset)
{
    struct AsmNode *store = arena_alloc(c->arena, sizeof(*store));                
    *store = (struct AsmNode){
        .kind = ASM_INSTRUCTION,
        .instruction = {
            .kind = ASM_I_STORE,
            .store = {
                .reg = reg,
                .address = address,
                .offset = offset,
            }
        }
    };
    return store;
}

struct AsmNode *generate_asm_set(struct CodeGen *c, struct Operand *reg, struct Operand *imm)
{
    struct AsmNode *set = arena_alloc(c->arena, sizeof(*set));                
    *set = (struct AsmNode){
        .kind = ASM_INSTRUCTION,
        .instruction = {
            .kind = ASM_I_SET,
            .set = {
                .reg = reg,
                .imm = imm,
            }
        }
    };
    return set;
}

struct AsmNode *generate_asm_unary(struct CodeGen *c, enum TokenKind operator, struct Operand *operand)
{
    struct AsmNode *unary = arena_alloc(c->arena, sizeof(*unary));
    *unary = (struct AsmNode){
        .kind = ASM_INSTRUCTION,
        .instruction = {
            .kind = ASM_I_UNARY,
            .unary = {
                .operator = operator,
                .operand = operand
            }
        }
    };
    return unary;
}

struct Operand *generate_asm_reg(struct CodeGen *c, enum Register reg_number)
{
    struct Operand *reg = arena_alloc(c->arena, sizeof(*reg));
    *reg = (struct Operand){
        .kind = OP_REGISTER,
        .reg = reg_number
    };
    return reg;
}

struct AsmNode *generate_asm_ret(struct CodeGen *c)
{
    struct AsmNode *ret = arena_alloc(c->arena, sizeof(*ret));
    *ret = (struct AsmNode){
        .kind = ASM_INSTRUCTION,
        .instruction = {
            .kind = ASM_I_RET
        }
    };
    return ret;
}



void generate_asm_ir(struct CodeGen *c, struct IRNode *node, struct AsmNodeList *instructions)
{
    if (node->kind == IR_INSTRUCTION)
    {
        if (node->instruction.kind == I_UNARY)
        {
            struct Operand *dst = ir_var_to_operand(c, node->instruction.i_unary.dst);
            struct Operand *src = ir_var_to_operand(c, node->instruction.i_unary.src);
            struct AsmNode *mov = generate_asm_mov(c, src, dst);
            struct AsmNode *unary = generate_asm_unary(c, node->instruction.i_unary.op, dst);
            LIST_PUSH(c->arena, instructions, mov, struct AsmNode);
            LIST_PUSH(c->arena, instructions, unary, struct AsmNode);
        }
        else if (node->instruction.kind == I_RETURN)
        {
            
            struct Operand *src = ir_var_to_operand(c, node->instruction.i_return.return_value);
            struct Operand *dst = generate_asm_reg(c, R5);
            
            struct AsmNode *mov = generate_asm_mov(c, src, dst);
            struct AsmNode *ret = generate_asm_ret(c);
            LIST_PUSH(c->arena, instructions, mov, struct AsmNode);
            LIST_PUSH(c->arena, instructions, ret, struct AsmNode);
        }
    }
}

struct StackTable *find_variable_offset(struct CodeGen *c, const char *name)
{
    for (int i = 0; i < c->n_variables; ++i)
    {
        if (!(strcmp(c->variables[i].name, name)))
        {
            return &(c->variables[i]);
        }
    }
    return NULL;
}

struct StackTable *add_variable(struct CodeGen *c, const char *name)
{
    c->variables[c->n_variables].name = name;
    c->variables[c->n_variables].offset = c->stack_offset;
    c->stack_offset += 2;
    c->n_variables += 1;
    return &(c->variables[c->n_variables-1]);
}

struct StackTable *ensure_variable(struct CodeGen *c, const char *pseudo)
{
    struct StackTable *variable = find_variable_offset(c, pseudo);

    if (!variable)
        variable = add_variable(c, pseudo);

    return variable;
}

void allocate_stack_variable(struct CodeGen *c, struct Operand *operand)
{
    operand->kind = OP_STACK;
    struct StackTable *variable = ensure_variable(c, operand->pseudo);
    operand->stack_offset = variable->offset;
}


struct AsmNode *generate_code(struct CodeGen *c, struct IRNode *ir)
{
    struct Operand *scratch_reg = arena_alloc(c->arena, sizeof(*scratch_reg));
    *scratch_reg = (struct Operand){
        .kind = OP_REGISTER,
        .reg = R0
    };
    struct Operand *stack_reg = arena_alloc(c->arena, sizeof(*stack_reg));
    *stack_reg = (struct Operand){
        .kind = OP_REGISTER,
        .reg = R7
    };

    if (ir->kind == IR_PROGRAM)
    {
        struct AsmNode *program = arena_alloc(c->arena, sizeof(*program));
        program->kind = ASM_PROGRAM;
        program->program.function = arena_alloc(c->arena, sizeof(*(program->program.function)));
        program->program.function->function.name = ir->program.function->function.name;
        program->program.function->function.instructions = arena_alloc(c->arena, sizeof(struct AsmNodeList));
        for (size_t i = 0; i < ir->program.function->function.instructions.count; ++i)
        {
            generate_asm_ir(c, ir->program.function->function.instructions.items[i], program->program.function->function.instructions);
        }

        // assigning stack offsets pass
        for (size_t i = 0; i < program->program.function->function.instructions->count; ++i)
        {
            struct AsmNode *instruction = program->program.function->function.instructions->items[i];
            
            if (instruction->instruction.kind == ASM_I_MOV)
            {
                if (instruction->instruction.mov.src->kind == OP_PSEUDO)
                {
                    allocate_stack_variable(c, instruction->instruction.mov.src);
                }
                if (instruction->instruction.mov.dst->kind == OP_PSEUDO)
                {
                    allocate_stack_variable(c, instruction->instruction.mov.dst);
                }
            }
            else if (instruction->instruction.kind == ASM_I_UNARY)
            {
                if (instruction->instruction.unary.operand->kind == OP_PSEUDO)
                {
                    allocate_stack_variable(c, instruction->instruction.unary.operand);
                }
            }
        }

        // inserting allocate_stack instruction and fixing illegal mov instructions
        struct AsmNodeList *fixed_instructions = arena_alloc(c->arena, sizeof(*fixed_instructions));
        struct AsmNode *allocate_stack = arena_alloc(c->arena, sizeof(*allocate_stack));
        *allocate_stack = (struct AsmNode){
            .kind = ASM_INSTRUCTION,
            .instruction = {
                .kind = ASM_I_ALLOCATE_STACK,
                .allocate_stack = {
                    .size = c->stack_offset
                }
            }
        };
        LIST_PUSH(c->arena, fixed_instructions, allocate_stack, struct AsmNode);

        for (size_t i = 0; i < program->program.function->function.instructions->count; ++i)
        {
            struct AsmNode *instruction = program->program.function->function.instructions->items[i];
            if (instruction->instruction.kind == ASM_I_MOV)
            {
                if (instruction->instruction.mov.src->kind == OP_STACK && instruction->instruction.mov.dst->kind == OP_STACK)
                {
                    struct AsmNode *load = generate_asm_load(c, scratch_reg, stack_reg, instruction->instruction.mov.src->stack_offset);
                    struct AsmNode *store = generate_asm_store(c, scratch_reg, stack_reg, instruction->instruction.mov.dst->stack_offset);
                    
                    LIST_PUSH(c->arena, fixed_instructions, load, struct AsmNode);
                    LIST_PUSH(c->arena, fixed_instructions, store, struct AsmNode);

                }
                else if (instruction->instruction.mov.src->kind == OP_IMM && instruction->instruction.mov.dst->kind == OP_STACK)
                {
                    struct AsmNode *set = generate_asm_set(c, scratch_reg, instruction->instruction.mov.src);
                    struct AsmNode *store = generate_asm_store(c, scratch_reg, stack_reg, instruction->instruction.mov.dst->stack_offset);
                    LIST_PUSH(c->arena, fixed_instructions, set, struct AsmNode);
                    LIST_PUSH(c->arena, fixed_instructions, store, struct AsmNode);
                }
                else if (instruction->instruction.mov.src->kind == OP_STACK && instruction->instruction.mov.dst->kind == OP_REGISTER)
                {
                    struct AsmNode *load = generate_asm_load(c, instruction->instruction.mov.dst, stack_reg, instruction->instruction.mov.src->stack_offset);
                    LIST_PUSH(c->arena, fixed_instructions, load, struct AsmNode);
                }
                else if (instruction->instruction.mov.src->kind == OP_IMM && instruction->instruction.mov.dst->kind == OP_REGISTER)
                {
                    struct AsmNode *set = generate_asm_set(c, instruction->instruction.mov.dst, instruction->instruction.mov.src);
                    LIST_PUSH(c->arena, fixed_instructions, set, struct AsmNode);
                }
                else
                {
                    LIST_PUSH(c->arena, fixed_instructions, instruction, struct AsmNode);
                }
            }
            else if (instruction->instruction.kind == ASM_I_UNARY)
            {
                if (instruction->instruction.unary.operand->kind == OP_STACK)
                {
                    struct AsmNode *load = generate_asm_load(c, scratch_reg, stack_reg, instruction->instruction.unary.operand->stack_offset);
                    struct AsmNode *unary = generate_asm_unary(c, instruction->instruction.unary.operator, scratch_reg);
                    struct AsmNode *store = generate_asm_store(c, scratch_reg, stack_reg, instruction->instruction.unary.operand->stack_offset);
                    LIST_PUSH(c->arena, fixed_instructions, load, struct AsmNode);
                    LIST_PUSH(c->arena, fixed_instructions, unary, struct AsmNode);
                    LIST_PUSH(c->arena, fixed_instructions, store, struct AsmNode);
                }
                else
                {
                    LIST_PUSH(c->arena, fixed_instructions, instruction, struct AsmNode);
                }
            }
            else
            {
                LIST_PUSH(c->arena, fixed_instructions, instruction, struct AsmNode);
            }
        }
        program->program.function->function.instructions = fixed_instructions;
        return program;
    }
}