#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define CHUNK_SIZE 512
#define STACK_SIZE 100

enum Direction { Forward, Backward };

char *find_match(char *start, char match, enum Direction dir) {
    char *ip = start;
    size_t count = 1;
    while (count) {
        dir == Forward ? ++ip : --ip;
        if (*ip == *start) ++count;
        if (*ip == match) --count;
    }
    return ip;
}

void preprocess_loops(char *loops[], char *program) {
    char *ip = program;
    while (*ip != 0) {
        if (*ip == '[') {
            loops[ip - program] = find_match(ip, ']', Forward);
        } else if (*ip == ']') {
            loops[ip - program] = find_match(ip, '[', Backward);
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

    char stack[STACK_SIZE] = {0};
    char *ptr = stack;
    char *loops[program_length];
    preprocess_loops(loops, program);

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
