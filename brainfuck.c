#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define CHUNK_SIZE 512
#define REGISTER_COUNT 100

void preprocess_loops(char *loops[], char *program, size_t program_length) {
    char *stack[program_length / 2]; // theoretically no more than half of the instructions can be '['
    char **top = stack;

    char *ip = program;
    while (*ip != 0) {
        if (*ip == '[') {
            *top = ip;
            ++top;
        } else if (*ip == ']') {
            --top;
            char *jump_start = *top;
            loops[ip - program] = jump_start;
            loops[jump_start - program] = ip;
        }
        ++ip;
    }
}

int main(int argc, char *argv[]) {
    char *program, *input;
    size_t program_length = 0;

    if (isatty(fileno(stdin))) {
        assert(argc > 1 && "Invalid number of arguments.");
        program = argv[1];
        program_length = strlen(program);
        if (argc > 2) input = argv[2];
    } else {
        char *tmp_program = malloc(CHUNK_SIZE);
        while (1) {
            ssize_t nb = read(fileno(stdin), tmp_program + program_length, CHUNK_SIZE);
            if (nb == 0) break;
            program_length += nb;
            tmp_program = realloc(tmp_program, program_length + CHUNK_SIZE);
        }
        program = tmp_program;
        if (argc > 1) input = argv[1];
    }

    char registers[REGISTER_COUNT] = {0};
    char *ptr = registers;
    char *loops[program_length];
    preprocess_loops(loops, program, program_length);

    char *ip = program;
    while (*ip != 0) {
        switch (*ip) {
        case '+':
            ++(*ptr);
            break;
        case '-':
            --(*ptr);
            break;
        case '>':
            ++ptr;
            break;
        case '<':
            --ptr;
            break;
        case '[':
            if (!*ptr) ip = loops[ip - program];
            break;
        case ']':
            if (*ptr) ip = loops[ip - program];
            break;
        case '.':
            printf("%c", *ptr);
            break;
        case ',':
            if (*input == 0) {
                *ptr = -1;
            } else {
                *ptr = *input;
                ++input;
            }
            break;
        }
        ++ip;
    }

    return EXIT_SUCCESS;
}
