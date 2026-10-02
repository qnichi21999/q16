#include "codegen.h"
#include "irgen.h"
#include "utils.h"
#include "stdio.h"
#include "string.h"
#include "emit.h"

uint8_t helper_flags = 0;

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

struct AsmNode *generate_asm_call(struct CodeGen *c, char *name)
{
    struct AsmNode *call = arena_alloc(c->arena, sizeof(*call));
    *call = (struct AsmNode){
        .kind = ASM_INSTRUCTION,
        .instruction = {
            .kind = ASM_I_CALL,
            .call = {
                .name = name
            }
        }
    };
    return call;
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

struct AsmNode *generate_asm_binary(struct CodeGen *c, enum TokenKind operator, struct Operand *loperand, struct Operand *roperand)
{
    struct AsmNode *binary = arena_alloc(c->arena, sizeof(*binary));
    *binary = (struct AsmNode){
        .kind = ASM_INSTRUCTION,
        .instruction = {
            .kind = ASM_I_BINARY,
            .binary = {
                .operator = operator,
                .loperand = loperand,
                .roperand = roperand
            }
        }
    };
    return binary;
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

struct AsmNode *generate_asm_label(struct CodeGen *c, struct IRNode *ir_label)
{
    struct AsmNode *label = arena_alloc(c->arena, sizeof(*label));
    *label = (struct AsmNode){
        .kind = ASM_INSTRUCTION,
        .instruction = {
            .kind = ASM_I_LABEL,
            .label.name = ir_label->instruction.i_label.name
        }
    };
    return label;
}

struct AsmNode *generate_asm_je(struct CodeGen *c, struct Operand *src, struct Operand *dst, struct AsmNode *target)
{
    struct AsmNode *je = arena_alloc(c->arena, sizeof(*je));
    *je = (struct AsmNode){
        .kind = ASM_INSTRUCTION,
        .instruction = {
            .kind = ASM_I_JE,
            .je = {
                .src = src,
                .dst = dst,
                .target = target
            }
        }
    };
    return je;
}

struct AsmNode *generate_asm_jg(struct CodeGen *c, struct Operand *src, struct Operand *dst, struct AsmNode *target)
{
    struct AsmNode *jg = arena_alloc(c->arena, sizeof(*jg));
    *jg = (struct AsmNode){
        .kind = ASM_INSTRUCTION,
        .instruction = {
            .kind = ASM_I_JG,
            .jg = {
                .src = src,
                .dst = dst,
                .target = target
            }
        }
    };
    return jg;
}


struct AsmNode *generate_asm_jmp(struct CodeGen *c, struct AsmNode *target)
{
    struct AsmNode *jmp = arena_alloc(c->arena, sizeof(*jmp));
    *jmp = (struct AsmNode){
        .kind = ASM_INSTRUCTION,
        .instruction = {
            .kind = ASM_I_JMP,
            .jmp = {
                .target = target
            }
        }
    };
    return jmp;
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
        else if (node->instruction.kind == I_BINARY)
        {
            if (node->instruction.i_binary.op == TK_PLUS || 
                node->instruction.i_binary.op == TK_MINUS || 
                node->instruction.i_binary.op == TK_SLASH ||
                node->instruction.i_binary.op == TK_PERCENT ||
                node->instruction.i_binary.op == TK_STAR)
            {
                struct Operand *src1 = ir_var_to_operand(c, node->instruction.i_binary.src1);
                struct Operand *src2 = ir_var_to_operand(c, node->instruction.i_binary.src2);
                struct Operand *dst = ir_var_to_operand(c, node->instruction.i_binary.dst);

                struct AsmNode *mov = generate_asm_mov(c, src1, dst);
                struct AsmNode *binary = generate_asm_binary(c, node->instruction.i_binary.op, src2, dst);
                LIST_PUSH(c->arena, instructions, mov, struct AsmNode);
                LIST_PUSH(c->arena, instructions, binary, struct AsmNode);
            }
        }
        else if (node->instruction.kind == I_COPY)
        {
            struct Operand *src = ir_var_to_operand(c, node->instruction.i_copy.src);
            struct Operand *dst = ir_var_to_operand(c, node->instruction.i_copy.dst);
            struct AsmNode *mov = generate_asm_mov(c, src, dst);
            LIST_PUSH(c->arena, instructions, mov, struct AsmNode);
        }
        else if (node->instruction.kind == I_JE)
        {
            struct Operand *src = ir_var_to_operand(c, node->instruction.i_je.src);
            struct Operand *dst = ir_var_to_operand(c, node->instruction.i_je.dst);
            struct AsmNode *je = generate_asm_je(c, src, dst, generate_asm_label(c, node->instruction.i_je.target));
            LIST_PUSH(c->arena, instructions, je, struct AsmNode);
        }
        else if (node->instruction.kind == I_JG)
        {
            struct Operand *src = ir_var_to_operand(c, node->instruction.i_jg.src);
            struct Operand *dst = ir_var_to_operand(c, node->instruction.i_jg.dst);
            struct AsmNode *jg = generate_asm_jg(c, src, dst, generate_asm_label(c, node->instruction.i_jg.target));
            LIST_PUSH(c->arena, instructions, jg, struct AsmNode);
        }
        else if (node->instruction.kind == I_JMP)
        {
            struct AsmNode *jmp = generate_asm_jmp(c, generate_asm_label(c, node->instruction.i_jmp.target));
            LIST_PUSH(c->arena, instructions, jmp, struct AsmNode);
        }
        else if (node->instruction.kind == I_JZ)
        {
            struct Operand *src = ir_var_to_operand(c, node->instruction.i_jg.src);
            struct Operand *dst = generate_asm_reg(c, R0);
            struct AsmNode *je = generate_asm_je(c, src, dst, generate_asm_label(c, node->instruction.i_jz.target));
            LIST_PUSH(c->arena, instructions, je, struct AsmNode);
        }
        else if (node->instruction.kind == I_LABEL)
        {
            struct AsmNode *label = generate_asm_label(c, node);
            LIST_PUSH(c->arena, instructions, label, struct AsmNode);
        }
    }
}

struct StackTable *find_variable_offset(struct CodeGen *c, const char *name)
{
    for (size_t i = 0; i < c->n_variables; ++i)
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
    struct Operand *reg1 = arena_alloc(c->arena, sizeof(*reg1));
    *reg1 = (struct Operand){
        .kind = OP_REGISTER,
        .reg = R1
    };
    struct Operand *reg2 = arena_alloc(c->arena, sizeof(*reg2));
    *reg2 = (struct Operand){
        .kind = OP_REGISTER,
        .reg = R2
    };
    struct Operand *reg3 = arena_alloc(c->arena, sizeof(*reg3));
    *reg3 = (struct Operand){
        .kind = OP_REGISTER,
        .reg = R3
    };
    struct Operand *reg4 = arena_alloc(c->arena, sizeof(*reg4));
    *reg4 = (struct Operand){
        .kind = OP_REGISTER,
        .reg = R4
    };
    
    struct Operand *scratch_reg = arena_alloc(c->arena, sizeof(*scratch_reg));
    *scratch_reg = (struct Operand){
        .kind = OP_REGISTER,
        .reg = R5
    };
    struct Operand *stack_reg = arena_alloc(c->arena, sizeof(*stack_reg));
    *stack_reg = (struct Operand){
        .kind = OP_REGISTER,
        .reg = R6
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
            else if (instruction->instruction.kind == ASM_I_BINARY)
            {
                if (instruction->instruction.binary.loperand->kind == OP_PSEUDO)
                {
                    allocate_stack_variable(c, instruction->instruction.binary.loperand);
                }
                if (instruction->instruction.binary.roperand->kind == OP_PSEUDO)
                {
                    allocate_stack_variable(c, instruction->instruction.binary.roperand);
                }
            }
            else if (instruction->instruction.kind == ASM_I_JE)
            {
                if (instruction->instruction.je.src->kind == OP_PSEUDO)
                {
                    allocate_stack_variable(c, instruction->instruction.je.src);
                }
                if (instruction->instruction.je.dst->kind == OP_PSEUDO)
                {
                    allocate_stack_variable(c, instruction->instruction.je.dst);
                }
            }
            else if (instruction->instruction.kind == ASM_I_JG)
            {
                if (instruction->instruction.jg.src->kind == OP_PSEUDO)
                {
                    allocate_stack_variable(c, instruction->instruction.jg.src);
                }
                if (instruction->instruction.jg.dst->kind == OP_PSEUDO)
                {
                    allocate_stack_variable(c, instruction->instruction.jg.dst);
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

        // legalization pass
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
            else if (instruction->instruction.kind == ASM_I_BINARY)
            {
                struct AsmNode *load_reg1 = NULL;
                struct AsmNode *load_reg2 = NULL;
                struct AsmNode *set_reg1 = NULL;
                struct AsmNode *set_reg2 = NULL;
                if (instruction->instruction.binary.roperand->kind == OP_STACK)
                {
                    load_reg1 = generate_asm_load(c, reg1, stack_reg, instruction->instruction.binary.roperand->stack_offset);
                    LIST_PUSH(c->arena, fixed_instructions, load_reg1, struct AsmNode);
                }
                else if (instruction->instruction.binary.roperand->kind == OP_IMM)
                {
                    set_reg1 = generate_asm_set(c, reg1, instruction->instruction.binary.roperand);
                    LIST_PUSH(c->arena, fixed_instructions, set_reg1, struct AsmNode);
                }
                if (instruction->instruction.binary.loperand->kind == OP_STACK)
                {
                    load_reg2 = generate_asm_load(c, reg2, stack_reg, instruction->instruction.binary.loperand->stack_offset);
                    LIST_PUSH(c->arena, fixed_instructions, load_reg2, struct AsmNode);
                }
                else if (instruction->instruction.binary.loperand->kind == OP_IMM)
                {
                    set_reg2 = generate_asm_set(c, reg2, instruction->instruction.binary.loperand);
                    LIST_PUSH(c->arena, fixed_instructions, set_reg2, struct AsmNode);
                }
                if (instruction->instruction.binary.operator == TK_SLASH)
                {
                    struct AsmNode *call_div = generate_asm_call(c, "__q16_div");
                    struct AsmNode *store_dst = generate_asm_store(c, scratch_reg, stack_reg, instruction->instruction.binary.roperand->stack_offset);
                    LIST_PUSH(c->arena, fixed_instructions, call_div, struct AsmNode);
                    LIST_PUSH(c->arena, fixed_instructions, store_dst, struct AsmNode);
                    helper_flags |= HELPER_DIV;
                    continue;
                }
                if (instruction->instruction.binary.operator == TK_PERCENT)
                {
                    struct AsmNode *call_mod = generate_asm_call(c, "__q16_mod");
                    struct AsmNode *store_dst = generate_asm_store(c, scratch_reg, stack_reg, instruction->instruction.binary.roperand->stack_offset);
                    LIST_PUSH(c->arena, fixed_instructions, call_mod, struct AsmNode);
                    LIST_PUSH(c->arena, fixed_instructions, store_dst, struct AsmNode);
                    helper_flags |= HELPER_DIV;
                    helper_flags |= HELPER_MOD;
                    continue;
                }
                if (instruction->instruction.binary.operator == TK_STAR)
                {
                    struct AsmNode *call_mul = generate_asm_call(c, "__q16_mul");
                    struct AsmNode *store_dst = generate_asm_store(c, scratch_reg, stack_reg, instruction->instruction.binary.roperand->stack_offset);
                    LIST_PUSH(c->arena, fixed_instructions, call_mul, struct AsmNode);
                    LIST_PUSH(c->arena, fixed_instructions, store_dst, struct AsmNode);
                    helper_flags |= HELPER_MUL;
                    continue;
                }
                struct AsmNode *binary = generate_asm_binary(c,
                                                    instruction->instruction.binary.operator,
                                                    (load_reg2 || set_reg2) ? reg2 : instruction->instruction.binary.loperand,
                                                    (load_reg1 || set_reg1) ? reg1 : instruction->instruction.binary.roperand);
                LIST_PUSH(c->arena, fixed_instructions, binary, struct AsmNode);
                if (load_reg1)
                {
                    struct AsmNode *store_reg1 = generate_asm_store(c, reg1, stack_reg, instruction->instruction.binary.roperand->stack_offset);
                    LIST_PUSH(c->arena, fixed_instructions, store_reg1, struct AsmNode);
                }
                if (load_reg2)
                {
                    struct AsmNode *store_reg2 = generate_asm_store(c, reg2, stack_reg, instruction->instruction.binary.loperand->stack_offset);
                    LIST_PUSH(c->arena, fixed_instructions, store_reg2, struct AsmNode);
                }
            }
            else if (instruction->instruction.kind == ASM_I_JE || instruction->instruction.kind == ASM_I_JG)
            {
                struct AsmNode *load_reg1 = NULL;
                struct AsmNode *load_reg2 = NULL;
                struct AsmNode *set_reg1 = NULL;
                struct AsmNode *set_reg2 = NULL;
                if (instruction->instruction.je.dst->kind == OP_STACK)
                {
                    load_reg1 = generate_asm_load(c, reg1, stack_reg, instruction->instruction.je.dst->stack_offset);
                    LIST_PUSH(c->arena, fixed_instructions, load_reg1, struct AsmNode);
                }
                else if (instruction->instruction.je.dst->kind == OP_IMM)
                {
                    set_reg1 = generate_asm_set(c, reg1, instruction->instruction.je.dst);
                    LIST_PUSH(c->arena, fixed_instructions, set_reg1, struct AsmNode);
                }
                if (instruction->instruction.je.src->kind == OP_STACK)
                {
                    load_reg2 = generate_asm_load(c, reg2, stack_reg, instruction->instruction.je.src->stack_offset);
                    LIST_PUSH(c->arena, fixed_instructions, load_reg2, struct AsmNode);
                }
                else if (instruction->instruction.je.src->kind == OP_IMM)
                {
                    set_reg2 = generate_asm_set(c, reg2, instruction->instruction.je.src);
                    LIST_PUSH(c->arena, fixed_instructions, set_reg2, struct AsmNode);
                }
                if (instruction->instruction.kind == ASM_I_JE)
                {
                    struct AsmNode *je = generate_asm_je(c,
                                                        (load_reg2 || set_reg2) ? reg2 : instruction->instruction.je.src,
                                                        (load_reg1 || set_reg1) ? reg1 : instruction->instruction.je.dst,
                                                        instruction->instruction.je.target);
                    LIST_PUSH(c->arena, fixed_instructions, je, struct AsmNode);
                }
                else
                {
                    struct AsmNode *jg = generate_asm_jg(c,
                                                        (load_reg2 || set_reg2) ? reg2 : instruction->instruction.jg.src,
                                                        (load_reg1 || set_reg1) ? reg1 : instruction->instruction.jg.dst,
                                                        instruction->instruction.jg.target);
                    LIST_PUSH(c->arena, fixed_instructions, jg, struct AsmNode);
                }
                if (load_reg1)
                {
                    struct AsmNode *store_reg1 = generate_asm_store(c, reg1, stack_reg, instruction->instruction.jg.dst->stack_offset);
                    LIST_PUSH(c->arena, fixed_instructions, store_reg1, struct AsmNode);
                }
                if (load_reg2)
                {
                    struct AsmNode *store_reg2 = generate_asm_store(c, reg2, stack_reg, instruction->instruction.jg.src->stack_offset);
                    LIST_PUSH(c->arena, fixed_instructions, store_reg2, struct AsmNode);
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
    return NULL;
}