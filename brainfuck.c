#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define CHUNK_SIZE 512

void preprocess_loops(int loops[], char *program, size_t program_length) {
    int stack[program_length / 2];
    int *top = stack;
    int n = 0;

    char *ip = program;
    while (*ip != 0) {
        if (*ip == '[') {
            *top = n;
            ++top;
            ++n;
        } else if (*ip == ']') {
            --top;
            int jump_start = *top;
            loops[n] = jump_start;
            loops[jump_start] = n;
            ++n;
        }
        ++ip;
    }
}

int main(int argc, char *argv[]) {
    char *program, *outfile;
    size_t program_length = 0;

    outfile = "bf.out";

    if (isatty(fileno(stdin))) {
        assert(argc > 1 && "Invalid number of arguments.");
        program = argv[1];
        program_length = strlen(program);
        if (argc > 2) outfile = argv[2];
    } else {
        char *tmp_program = malloc(CHUNK_SIZE);
        while (1) {
            ssize_t nb = read(fileno(stdin), tmp_program + program_length, CHUNK_SIZE);
            if (nb == 0) break;
            program_length += nb;
            tmp_program = realloc(tmp_program, program_length + CHUNK_SIZE);
        }
        program = tmp_program;
        if (argc > 1) outfile = argv[1];
    }

    int loops[program_length];
    preprocess_loops(loops, program, program_length);

    FILE *fd = fopen(outfile, "w+");
    assert(fd > 0);

    // r12 = register pointer
    // r12 = input pointer
    const char *header =
        "    .globl _start"        "\n"
        "    .text"                "\n"
        "_start:"                  "\n"
        "    xorq %r13, %r13"      "\n"
        "    movq (%rsp), %rax"    "\n"
        "    cmpq $1, %rax"        "\n"
        "    jle no_args"          "\n"
        "    movq 16(%rsp), %r13"  "\n"
        "no_args:"                 "\n"
        "    movq %rsp, %rbp"      "\n"
        "    subq $100, %rsp"      "\n"
        "    movq %rsp, %r12"      "\n"
        "    jmp main"             "\n"
        ""                         "\n"
        ;

    size_t bn = fwrite(header, strlen(header), 1, fd);
    assert(bn > 0);

    char *proc_io_in =
        "get_input:"            "\n"
        "    movq (%r13), %rax" "\n"
        "    testb %al, %al"    "\n"
        "    jz io_empty"       "\n"
        "    movb %al, (%r12)"  "\n"
        "    incq %r13"         "\n"
        "    jmp io_end"        "\n"
        "io_empty:"             "\n"
        "    movb $0, (%r12)"   "\n"
        "io_end:"               "\n"
        "    ret"               "\n"
        ""                      "\n"
        "main:"                 "\n"
        ;
    bn = fwrite(proc_io_in, strlen(proc_io_in), 1, fd);
    assert(bn > 0);

    char label_conv[10];
    int label = 0;
    char *ip = program;
    while (*ip != 0) {
        switch (*ip) {
        case '+': {
            char *out = "    incb (%r12)" " # +" "\n";
            size_t bn = fwrite(out, strlen(out), 1, fd);
            assert(bn > 0);
        } break;
        case '-': {
            char *out = "    decb (%r12)" " # -" "\n";
            size_t bn = fwrite(out, strlen(out), 1, fd);
            assert(bn > 0);
        } break;
        case '>': {
            char *out = "    incq %r12" " # >" "\n";
            size_t bn = fwrite(out, strlen(out), 1, fd);
            assert(bn > 0);
        } break;
        case '<': {
            char *out = "    decq %r12" " # <" "\n";
            size_t bn = fwrite(out, strlen(out), 1, fd);
            assert(bn > 0);
        } break;
        case '[': {
            size_t bn;
            int nl;

            {
                char *out =
                    "    movq (%r12), %rax" " # [" "\n"
                    "    testb %al, %al"          "\n"
                    ;
                bn = fwrite(out, strlen(out), 1, fd);
                assert(bn > 0);
            }

            {
                bn = fwrite("    jz cond_", 12, 1, fd);
                assert(bn > 0);

                int nl = snprintf(label_conv, 8, "%d", loops[label]);
                label_conv[nl] = '\n';
                bn = fwrite(label_conv, nl+1, 1, fd);
                assert(bn > 0);
            }

            {
                bn = fwrite("cond_", 5, 1, fd);
                assert(bn > 0);

                int nl = snprintf(label_conv, 8, "%d", label);
                label_conv[nl] = ':';
                label_conv[nl+1] = '\n';
                bn = fwrite(label_conv, nl+2, 1, fd);
                assert(bn > 0);
            }

            ++label;
        } break;
        case ']': {
            size_t bn;
            int nl;

            {
                char *out =
                    "    movq (%r12), %rax" " # ]" "\n"
                    "    testb %al, %al"          "\n"
                    ;
                bn = fwrite(out, strlen(out), 1, fd);
                assert(bn > 0);
            }

            {
                bn = fwrite("    jnz cond_", 13, 1, fd);
                assert(bn > 0);

                int nl = snprintf(label_conv, 8, "%d", loops[label]);
                label_conv[nl] = '\n';
                bn = fwrite(label_conv, nl+1, 1, fd);
                assert(bn > 0);
            }

            {
                bn = fwrite("cond_", 5, 1, fd);
                assert(bn > 0);

                int nl = snprintf(label_conv, 8, "%d", label);
                label_conv[nl] = ':';
                label_conv[nl+1] = '\n';
                bn = fwrite(label_conv, nl+2, 1, fd);
                assert(bn > 0);
            }

            ++label;
        } break;
        case '.': {
            char *out =
                "    movq $1, %rax"  " # ." "\n"
                "    movq $1, %rdi"        "\n"
                "    movq %r12, %rsi"      "\n"
                "    movq $1, %rdx"        "\n"
                "    syscall"              "\n"
                ;
            size_t bn = fwrite(out, strlen(out), 1, fd);
            assert(bn > 0);
        } break;
        case ',': {
            char *out = "    call get_input" " # ," "\n";
            size_t bn = fwrite(out, strlen(out), 1, fd);
            assert(bn > 0);
        } break;
        }
        ++ip;
    }

    char *footer =
        ""                   "\n"
        "    movq $60, %rax" "\n"
        "    movq $0, %rdi"  "\n"
        "    syscall"        "\n"
        ;
    bn = fwrite(footer, strlen(footer), 1, fd);
    assert(bn > 0);

    assert(fclose(fd) == 0);

    assert(system("as bf.out -o bf.o -g") == 0);
    assert(system("ld bf.o -o bf") == 0);

    return EXIT_SUCCESS;
}
