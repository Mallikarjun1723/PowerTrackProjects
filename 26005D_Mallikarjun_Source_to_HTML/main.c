/*
Name : Malliakrjun Iranna Bandai
Date : 20-10-2026
Project : Lexical Analyser  ->  Source to HTML
Description :A lightweight lexer for C source code, written in C. It reads a .c file, breaks it into tokens (keywords, identifiers, numbers, strings, operators, symbols and header lines), and writes each one into an html file. The colours are not in the html, they are in a separate file styles.css. Comments are written as comments, and lexical errors are caught and counted, including invalid numbers, bad string or char constants, unterminated comments and unmatched brackets.

Usage   : ./lexer <source.c> [output name]
Example : ./lexer test.c        -> creates test.c.html
          ./lexer test.c abc    -> creates abc.html
*/
#include <stdio.h>
#include "lexical.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source.c>\n", argv[0]);
        fprintf(stderr, "Example : %s test.c\n", argv[0]);
        return 1;
    }

    lexer lx = {0};              /* zeroes every counter and the buffer */
    lx.line = 1;
    char dest_file[100];   /*html file name*/
    lx.fd = fopen(argv[1], "r");
    if (lx.fd == NULL) {
        perror(argv[1]);
        return 1;
    }
       /* to check the output name is given or not */
      snprintf(dest_file, sizeof(dest_file), "%s.html", (argc > 2) ? argv[2] : argv[1]);
 
    /* open the html file */
    lx.out = fopen(dest_file, "w");
    if (lx.out == NULL) {
        perror(dest_file);
        fclose(lx.fd);
        return 1;
    }
 
    html_begin(lx.out, argv[1]);     /* html starting tags */
    lexer_run(&lx);                  /* every token is written by emit() */
    html_end(lx.out);                /* html closing tags */
 
    fclose(lx.fd);
    fclose(lx.out);
    printf("HTML file created : %s\n", dest_file);
 
    return lx.error_cnt ? 1 : 0;
}
