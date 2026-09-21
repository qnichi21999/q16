#include <stdio.h>
#include <stdint.h>
#include "../../common/q16.h"
#include "lexer.h"
#include <string.h>
#include "compiler.h"
#include "arena.h"
#include "parser.h"
#include "irgen.h"
#include "utils.h"
#include "codegen.h"
#include "emit.h"


int main(int argc, char *argv[]) {
    if (argc == 1)
    {
        fprintf(stderr, "Provide input file as an argument");
        return 1;
    }
    char *input_path = NULL;
    char *output_path = NULL;
 
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_path = argv[++i];
            } else {
                fprintf(stderr, "Error: -o requires an argument.\n");
                return 1;
            }
        } else {
            input_path = argv[i];
        }
    }

    if (!input_path || !output_path) {
        fprintf(stderr, "Usage: %s <inputfile> -o <outputfile>\n", argv[0]);
        return 1;
    }
    
    FILE *input_file = fopen(input_path, "r");
    if (input_file == NULL) {
        perror("Error opening input file");
        return 1;
    }

    FILE *output_file = fopen(output_path, "w");
    if (output_file == NULL) {
        perror("Error opening output file");
        fclose(output_file);
        return 1;
    }

    struct Token tokens[65536];
    char input[65536];

    struct Context ctx;
    ctx.ast_arena = (struct Arena) {
            .data = ctx.memory,
            .capacity = AST_ARENA_SIZE,
            .used = 0
        };
    ctx.ir_arena = (struct Arena) {
        .data = ctx.memory+AST_ARENA_SIZE,
        .capacity = IR_ARENA_SIZE,
        .used = 0
    };
    ctx.rodata_arena = (struct Arena) {
            .data = ctx.memory+AST_ARENA_SIZE+IR_ARENA_SIZE,
            .capacity = RODATA_ARENA_SIZE,
            .used = 0
        };
    ctx.temp_arena = (struct Arena) {
        .data = ctx.memory+AST_ARENA_SIZE+IR_ARENA_SIZE+RODATA_ARENA_SIZE,
        .capacity = TEMP_ARENA_SIZE,
        .used = 0
    };

    printf("arena data=%p capacity=%zu used=%zu\n",
       (void *)ctx.ast_arena.data,
       ctx.ast_arena.capacity,
       ctx.ast_arena.used);


    size_t len = fread(input, sizeof(char), 65536, input_file);
    input[len] = '\0';

    size_t token_count = tokenize(input, tokens, &ctx);
    print_tokens(tokens, token_count);

    struct Parser parser;
    parser.arena = &(ctx.ast_arena);
    parser.tokens = tokens;
    parser.count = token_count;

    struct AstNode *ast = parse(&parser);
    print_ast(ast, 0);

    struct IRGen irgen;
    irgen.arena = &(ctx.ir_arena);
    struct IRNode *ir = generate_ir(&irgen, ast);
    print_ir(ir, 0);

    struct CodeGen codegen;
    codegen.arena = &(ctx.ir_arena);
    struct AsmNode *asm_ir = generate_code(&codegen, ir);
    print_asm_ir(asm_ir, 0);

    emit_asm(output_path, asm_ir);

    return 0;
}
