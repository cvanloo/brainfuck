#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define CHUNK_SIZE 512
#define STACK_SIZE 100
#define MAX_LOOPS 10

int main(int argc, char *argv[]) {
    char *program, *input;

    if (isatty(fileno(stdin))) {
        assert(argc > 1 && "Invalid number of arguments.");
        program = argv[1];
        if (argc > 2) input = argv[2];
    } else {
        char *tmp_program = malloc(CHUNK_SIZE);
        size_t total_read = 0;
        while (1) {
            ssize_t nb = read(fileno(stdin), tmp_program + total_read, CHUNK_SIZE);
            if (nb == 0) break;
            total_read += nb;
            tmp_program = realloc(tmp_program, total_read + CHUNK_SIZE);
        }
        program = tmp_program;
        if (argc > 1) input = argv[1];
    }

    char stack[STACK_SIZE] = {0};
    char *ptr = stack;

    char *loops[MAX_LOOPS];
    int8_t current_loop = -1;
    char *instruction = program;
    while (*instruction != 0) {
        switch (*instruction) {
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
            if (*ptr == 0) {
                size_t top = 1;
                for (size_t i = 0; i < top; ++i) {
                    while (*(++instruction) != ']')
                        if (*instruction == '[') ++top;
                }
                // Continue executing after loop end (++instruction after switch end).
            } else {
                ++current_loop;
                assert(current_loop < MAX_LOOPS && "Exhausted loop stack.");
                loops[current_loop] = instruction;
            }
            break;
        case ']':
            if (*ptr != 0) {
                instruction = loops[current_loop];
                // Continue executing after loop start (++instruction after switch end).
            } else {
                --current_loop;
            }
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
        ++instruction;
    }

    return EXIT_SUCCESS;
}
