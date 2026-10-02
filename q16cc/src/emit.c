#include "codegen.h"
#include "stdio.h"
#include "stdlib.h"
#include "lexer.h"
#include <libgen.h>
#include "emit.h"


void emit_allocate_stack(FILE *f, uint16_t size, size_t tab_count)
{
    fprintf(f, "SET R5, %d\n", size);
    for (size_t j = 0; j < tab_count; ++j)
        fprintf(f, "    ");
    fprintf(f, "SUB R6, R5\n");
}

void emit_load(FILE *f, struct Operand *dst, struct Operand *address, uint16_t imm)
{
    fprintf(f, "LOAD R%d, [R%d], %d\n", dst->reg, address->imm, imm);
}

void emit_store(FILE *f, struct Operand *src, struct Operand *address, uint16_t imm)
{
    fprintf(f, "STORE R%d, [R%d], %d\n", src->reg, address->imm, imm);
}

void emit_set(FILE *f, struct Operand *dst, uint16_t imm)
{
    fprintf(f, "SET R%d, %d\n", dst->reg, imm);
}

void emit_ret(FILE *f)
{
    fprintf(f, "RET\n");
}

void emit_unary_neg(FILE *f, struct Operand *reg, size_t tab_count)
{
    fprintf(f, "SUB R0, R%d\n", reg->reg);
    for (size_t j = 0; j < tab_count; ++j)
        fprintf(f, "    ");
    fprintf(f, "MOV R%d, R0\n", reg->reg);
    for (size_t j = 0; j < tab_count; ++j)
        fprintf(f, "    ");
    fprintf(f, "SET R0, 0\n");
}

// operands are inverted in binary operations because dont ask
void emit_binary_add(FILE *f, struct Operand *loperand, struct Operand *roperand)
{
    fprintf(f, "ADD R%d, R%d\n", roperand->reg, loperand->reg);
}

void emit_binary_sub(FILE *f, struct Operand *loperand, struct Operand *roperand)
{
    fprintf(f, "SUB R%d, R%d\n", roperand->reg, loperand->reg);
}

void emit_mov(FILE *f, struct Operand *src, struct Operand *dst)
{
    fprintf(f, "MOV R%d, R%d\n", src->reg, dst->reg);
}

void emit_call(FILE *f, char *name)
{
    fprintf(f, "CALL %s\n", name);
}

void emit_halt(FILE *f)
{
    fprintf(f, "HALT\n");
}

void emit_je(FILE *f, struct Operand *src, struct Operand *dst, struct AsmNode *target)
{
    fprintf(f, "JE R%d, R%d, %s\n", dst->reg, src->reg, target->instruction.label.name);
}

void emit_jg(FILE *f, struct Operand *src, struct Operand *dst, struct AsmNode *target)
{
    fprintf(f, "JG R%d, R%d, %s\n", dst->reg, src->reg, target->instruction.label.name);
}

void emit_jmp(FILE *f, struct AsmNode *target)
{
    fprintf(f, "JMP %s\n", target->instruction.label.name);
}

void emit_label(FILE *f, struct AsmNode *label)
{
    fprintf(f, "%s:\n", label->instruction.label.name);
}

void write_helper(FILE *f, const char *helper_path)
{
    FILE *input_file = fopen(helper_path, "r");
    if (!input_file)
    {
        fprintf(stderr, "Error: cannot open %s\n", helper_path);
        return;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), input_file))
    {
        fputs(buffer, f);
    }

    fclose(input_file);
}

void emit_helpers(FILE *f)
{
    if (helper_flags)
        fprintf(f, "\n\n\n");

    if (helper_flags & HELPER_DIV)
    {
        write_helper(f, "q16asm_helpers/q16_div.asm");
    }
    if (helper_flags & HELPER_MOD)
    {
        write_helper(f, "q16asm_helpers/q16_mod.asm");
    }
    if (helper_flags & HELPER_MUL)
    {
        write_helper(f, "q16asm_helpers/q16_mul.asm");
    }
}

void emit_asm(char *output_path, struct AsmNode *node)
{
    char *filename = basename(output_path);
    size_t tab_count = 0;

    FILE *output_file = fopen(filename, "w");
    if (output_file == NULL) {
        perror("Error opening output file");
        fclose(output_file);
        return;
    }

    if (node->kind != ASM_PROGRAM)
    {
        printf("WTF DUDE HOW?!");
        abort();
    }


    fprintf(output_file, "_%.*s:\n", (int)node->program.function->function.name->len, node->program.function->function.name->start);
    tab_count++;
    for (size_t i = 0; i < node->program.function->function.instructions->count; ++i)
    {
        for (size_t j = 0; j < tab_count; ++j)
            fprintf(output_file, "    ");
        struct AsmNode *instruction = node->program.function->function.instructions->items[i];
        if (instruction->instruction.kind == ASM_I_ALLOCATE_STACK)
        {
            emit_allocate_stack(output_file, instruction->instruction.allocate_stack.size, tab_count);
        }
        else if (instruction->instruction.kind == ASM_I_LOAD)
        {
            emit_load(output_file, instruction->instruction.load.reg, instruction->instruction.load.address, instruction->instruction.load.offset);
        }
        else if (instruction->instruction.kind == ASM_I_STORE)
        {
            emit_store(output_file, instruction->instruction.store.reg, instruction->instruction.store.address, instruction->instruction.store.offset);
        }
        else if (instruction->instruction.kind == ASM_I_SET)
        {
            emit_set(output_file, instruction->instruction.set.reg, instruction->instruction.set.imm->imm);
        }
        else if (instruction->instruction.kind == ASM_I_RET)
        {
            emit_ret(output_file);
        }
        else if (instruction->instruction.kind == ASM_I_CALL)
        {
            emit_call(output_file, instruction->instruction.call.name);
        }
        else if (instruction->instruction.kind == ASM_I_MOV)
        {
            emit_mov(output_file, instruction->instruction.mov.src, instruction->instruction.mov.dst);
        }
        else if (instruction->instruction.kind == ASM_I_UNARY)
        {
            if (instruction->instruction.unary.operator == TK_MINUS)
            {
                emit_unary_neg(output_file, instruction->instruction.unary.operand, tab_count);
            }
        }
        else if (instruction->instruction.kind == ASM_I_BINARY)
        {
            if (instruction->instruction.binary.operator == TK_PLUS)
            {
                emit_binary_add(output_file, instruction->instruction.binary.loperand, instruction->instruction.binary.roperand);
            }
            else if (instruction->instruction.binary.operator == TK_MINUS)
            {
                emit_binary_sub(output_file, instruction->instruction.binary.loperand, instruction->instruction.binary.roperand);
            }
        }
        else if (instruction->instruction.kind == ASM_I_JE)
        {
            emit_je(output_file, instruction->instruction.je.src, instruction->instruction.je.dst, instruction->instruction.je.target);
        }
        else if (instruction->instruction.kind == ASM_I_JG)
        {
            emit_jg(output_file, instruction->instruction.jg.src, instruction->instruction.jg.dst, instruction->instruction.jg.target);
        }
        else if (instruction->instruction.kind == ASM_I_JMP)
        {
            emit_jmp(output_file, instruction->instruction.jmp.target);
        }
        else if (instruction->instruction.kind == ASM_I_LABEL)
        {
            emit_label(output_file, instruction);
        }
    }
    emit_halt(output_file);
    emit_helpers(output_file);

    fclose(output_file);
    
}