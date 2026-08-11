#include <stdio.h>
#include <stdint.h>
#include "../../common/q16.h"
#include "lexer.h"
#include "assembler.h"
#include "preprocessor.h"
#include <string.h>

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
        fclose(input_file);
        return 1;
    }

    struct Context ctx = {0};
    ctx.base_address = 0x0400;
    struct Token tokens[65536];
    char input[65536];
    fread(input, sizeof(char), 65536, input_file);
    tokenize(input, tokens);

    struct Token line[32];

    // preprocess
    for(int i, n = 0; i < 65536; ++i)
    {
        if (tokens[i].kind == TK_EOF)
        {
            tokens_pp[n_tokens_pp] = tokens[i];
            n_tokens_pp++;
            break;
        } 
        if (tokens[i].kind == TK_EOL)
        {
            preprocess_line(&ctx, line, n);
            memset(line, 0, sizeof(line));
            n = 0;
            tokens_pp[n_tokens_pp] = tokens[i];
            n_tokens_pp++;
        }
        else
        {
           line[n] = tokens[i];
           n++; 
        }
    }

    // assemble
    for(int i, n = 0; i < 65536; ++i)
    {
        if (tokens_pp[i].kind == TK_EOF) break;
        if (tokens_pp[i].kind == TK_EOL)
        {
            line[n] = tokens_pp[i];
            assemble_line(&ctx, line);
            memset(line, 0, sizeof(line));
            n = 0;
        }
        else
        {
           line[n] = tokens_pp[i];
           n++; 
        }
    }

    resolve_patches(&ctx);
    printf("Total: %zu bytes of code\n", ctx.out_size);

    uint8_t entry[2] = { 0x04, 0x00 };
    fwrite(entry, 1, 2, output_file);
    fwrite(ctx.out, 1, ctx.out_size, output_file);
    fclose(output_file);

    printf("Wrote %zu bytes to %s\n", ctx.out_size + 2, output_path);

    return 0;
}
