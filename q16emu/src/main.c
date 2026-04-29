#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include "globals.h"
#include "../../common/q16.h"

uint16_t fetch(CPU *c)
{
    uint16_t word = (c->memory[c->pc]<<8 | c->memory[c->pc+1]);
    c->pc += 2;
    return word;
}

void execute(CPU *c, uint16_t word)
{
    uint8_t op = word >> 8;
    uint8_t reg_a = (word >> 4) & 0x0F;
    uint8_t reg_b = word & 0x0F;
    switch(op) {
        case OP_HALT:
            c->halted = 1;
            break;
        case OP_SET:
            c->r[reg_a] = fetch(c);
            break;
        case OP_MOV:
            c->r[reg_a] = c->r[reg_b];
            break;
        case OP_LOAD:
            c->r[reg_a] = (c->memory[c->r[reg_b]]<<8) | c->memory[c->r[reg_b]+1];
            break;
        case OP_STORE:
            c->memory[c->r[reg_a]] = c->r[reg_b] >> 8 & 0xFF;
            c->memory[c->r[reg_a]+1] = c->r[reg_b] & 0xFF;
            break;
        case OP_ADD:
            c->r[reg_a] += c->r[reg_b];
            break;
        case OP_SUB:
            c->r[reg_a] -= c->r[reg_b];
            break;
        case OP_AND:
            c->r[reg_a] &= c->r[reg_b];
            break;
        case OP_OR:
            c->r[reg_a] |= c->r[reg_b];
            break;
        case OP_LSH:
            c->r[reg_a] <<= fetch(c);
            break;
        case OP_JMP:
            c->pc = fetch(c);
            break;
        case OP_JE:
        {
            uint16_t addr = fetch(c);
            if (c->r[reg_a] == c->r[reg_b])
            {
                c->pc = addr;
            }
            break;
        }
            
        case OP_JG:
        {
            uint16_t addr = fetch(c);
            if (c->r[reg_a] > c->r[reg_b])
            {
                c->pc = addr;
            }
            break;
        }
        case OP_PUSH:
            c->sp -= 2;
            c->memory[c->sp] = (c->r[reg_a] >> 8) & 0xFF;
            c->memory[c->sp+1] = c->r[reg_a] & 0xFF;
            break;
        case OP_POP:
            c->r[reg_a] = (c->memory[c->sp] << 8) | c->memory[c->sp+1];
            c->sp += 2;
            break;
        case OP_CALL:
        {
            uint16_t dest = fetch(c);
            c->sp -= 2;
            c->memory[c->sp] = (c->pc >> 8) & 0xFF;
            c->memory[c->sp+1] = c->pc & 0xFF;
            c->pc = dest;
            break;
        }
            
        case OP_RET:
            c->pc = (c->memory[c->sp] << 8) | c->memory[c->sp+1];
            c->sp += 2;
            break;
        case OP_OUT:
            printf("R%d: %d\n", reg_a, c->r[reg_a]);
            break;
    }
}

void saveToFile(const char *filename, uint8_t *program, size_t size) {
    FILE *f = fopen(filename, "wb");
    if (f == NULL)
    {
        printf("Cannot open file \"%s\" for writing, aborting");
        exit(1);
    }

    uint8_t entry_point[2] = {0x04, 0x00};
    fwrite(entry_point, 1, 2, f);

    fwrite(program, 1, size, f);

    fclose(f);
    printf("Wrote %zu bytes to \"%s\"\n", size + 2, filename);
}

void load_executable(CPU *c, char *filepath)
{
    FILE *f = fopen(filepath, "rb");

    if(f == NULL)
    {
        printf("Cannot open file \"%s\", aborting", filepath);
        exit(1);
    }

    uint8_t entry_point[2];
    uint8_t byte;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    fread(&entry_point, 1, 2, f);
    c->memory[0x0000] = entry_point[0];
    c->memory[0x0001] = entry_point[1];

    fread(c->memory+TEXT_START, 1, size-2, f);
}


int main(int argc, char *argv[])
{
    CPU cpu = {0};
    load_executable(&cpu, argv[1]);
    
    cpu.pc = (cpu.memory[0x0000] << 8) | cpu.memory[0x0001];
    cpu.sp = 0xFFFF;
    while (!cpu.halted)
    {
        execute(&cpu, fetch(&cpu));
    }
    return 0;
}
