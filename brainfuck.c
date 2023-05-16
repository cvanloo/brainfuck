#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define CHUNK_SIZE 512

int main(int argc, char *argv[]) {
    char *program, *input;

    if (isatty(fileno(stdin))) {
        assert(argc > 1 && "Invalid number of arguments.");
        program = argv[1];
        if (argc > 2) {
            input = argv[2];
        }
    } else {
        char *tmp_program = malloc(CHUNK_SIZE);
        size_t total_read = 0;
        while (1) {
            ssize_t nb = read(fileno(stdin), tmp_program + total_read, CHUNK_SIZE);
            if (nb == 0)
                break;
            total_read += nb;
            tmp_program = realloc(tmp_program, total_read + CHUNK_SIZE);
        }
        program = tmp_program;

        if (argc > 1) {
            input = argv[1];
        }
    }

    char stack[100] = {0};
    size_t ptr = 0;

    char *loops[10];
    int8_t current_loop = -1;
    char *instruction = program;
    while (*instruction != 0) {
        switch (*instruction) {
        case '+':
            ++stack[ptr];
            break;
        case '-':
            --stack[ptr];
            break;
        case '>':
            ++ptr;
            break;
        case '<':
            --ptr;
            break;
        case '[':
            if (stack[ptr] == 0) {
                size_t top = 1;
                for (size_t i = 0; i < top; ++i) {
                    while (*(++instruction) != ']') {
                        if (*instruction == '[') {
                            ++top;
                        }
                    }
                }
            } else {
                ++current_loop;
                loops[current_loop] = instruction;
            }
            break;
        case ']':
            if (stack[ptr] != 0) {
                instruction = loops[current_loop];
            } else {
                --current_loop;
            }
            break;
        case '.':
            printf("%c", stack[ptr]);
            break;
        case ',':
            if (*input == 0) {
                stack[ptr] = -1;
            } else {
                stack[ptr] = *input;
                ++input;
            }
            break;
        }
        ++instruction;
    }

    return EXIT_SUCCESS;
}
