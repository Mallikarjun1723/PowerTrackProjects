/*
Name : Malliakrjun Iranna Bandai
Date : 20-10-2026
Project : Lexical Analyser
Description :A lightweight lexer for C source code, written in C. It reads a .c file, breaks it into tokens (keywords, identifiers, numbers, strings, operators, symbols and header lines), and prints each one with its line number and type. Comments are skipped, and lexical errors are caught and counted, including invalid numbers, bad string or char constants, unterminated comments and unmatched brackets.
*/

#include <stdio.h>
#include "lexical.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source.c>\n", argv[0]);
        return 1;
    }

    lexer lx = {0};              /* zeroes every counter and the buffer */
    lx.line = 1;
    lx.fd = fopen(argv[1], "r");
    if (lx.fd == NULL) {
        perror(argv[1]);
        return 1;
    }

    lexer_run(&lx);
    fclose(lx.fd);

    return lx.error_cnt ? 1 : 0;
}
