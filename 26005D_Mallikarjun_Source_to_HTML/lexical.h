#ifndef LEXICAL_H
#define LEXICAL_H

#include <stdio.h>

#define WORD_BUFF_SIZE 128

#define RESET       "\x1b[0m"
#define RED         "\x1b[31m"

typedef enum {
   KEYWORD_DATA,       
   KEYWORD_NON_DATA,   
    IDENTIFIER,         
    NUMERIC_CONST,      
    FLOAT_CONST,        
    STRING,            
    CHAR_CONST,        
    OPERATOR,           
    SYMBOL,             
    HEADER_FILE,
    ERROR_TOKEN 
} token_type;

typedef struct {
    FILE *fd;
    FILE *out;    /*html file*/
    int   line;
    char  word[WORD_BUFF_SIZE];     /* token being built */
    int   word_idx;
    int   brace_count, bracket_count, paren_count;
    int   error_cnt;
} lexer;

void lexer_run(lexer *lx);                      /* main loop + decides which scanner to call */
void scan_word(lexer *lx, int first);           /* keyword or identifier */
void scan_number(lexer *lx, int first);         /* integer or float, includes validation */
void scan_quoted(lexer *lx, int quote);         /* string ("), char constant (') */
void scan_comment(lexer *lx);                   /* single-line and multi-line */
void scan_operator(lexer *lx, int first);       /* one or two character operators */
void emit(lexer *lx, token_type type, const char *text);  /* print a token OR an error */
int  lookup(char *table[], const char *word);     /* keyword table search */
void html_begin(FILE *out, const char *title);  /* html starting tags */
void html_end(FILE *out);                       /* html closing tags  */
#endif
