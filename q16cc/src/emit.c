#include "codegen.h"
#include "stdio.h"
#include "stdlib.h"
#include "lexer.h"
#include <libgen.h>


void emit_allocate_stack(FILE *f, uint16_t size, size_t tab_count)
{
    fprintf(f, "SET R0, %d\n", size);
    for (size_t j = 0; j < tab_count; ++j)
        fprintf(f, "    ");
    fprintf(f, "SUB R7, R0\n");
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

void emit_neg(FILE *f, struct Operand *reg, size_t tab_count)
{
    uint8_t scratch = reg->reg == R0 ? 1 : 0;
    fprintf(f, "SET R%d, 0\n", scratch);
    for (size_t j = 0; j < tab_count; ++j)
        fprintf(f, "    ");
    fprintf(f, "SUB R%d, R%d\n", scratch, reg->reg);
    for (size_t j = 0; j < tab_count; ++j)
        fprintf(f, "    ");
    fprintf(f, "MOV R%d, R%d\n", reg->reg, scratch);

}

void emit_mov(FILE *f, struct Operand *src, struct Operand *dst)
{
    fprintf(f, "MOV R%d, R%d\n", src->reg, dst->reg);
}

void emit_asm(const char *output_path, struct AsmNode *node)
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


    fprintf(output_file, ".%.*s\n", node->program.function->function.name->len, node->program.function->function.name->start);
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
        else if (instruction->instruction.kind == ASM_I_MOV)
        {
            emit_mov(output_file, instruction->instruction.mov.src, instruction->instruction.mov.dst);
        }
        else if (instruction->instruction.kind == ASM_I_UNARY)
        {
            if (instruction->instruction.unary.operator == TK_MINUS)
            {
                emit_neg(output_file, instruction->instruction.unary.operand, tab_count);
            }
        }
    }

    fclose(output_file);
    
}