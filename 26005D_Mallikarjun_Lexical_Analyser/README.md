# Lexical Analyser in C

Batch: 26005D
Student: Mallikarjun

A lightweight lexer for C source code, written in C. It reads a .c file, breaks it into tokens (keywords, identifiers, numbers, strings, operators, symbols and header lines), and prints each one with its line number and type. Comments are skipped, and lexical errors are caught and counted, including invalid numbers, bad string or char constants, unterminated comments and unmatched brackets.

## Usage
gcc main.c lexer.c
./a.out test.c
