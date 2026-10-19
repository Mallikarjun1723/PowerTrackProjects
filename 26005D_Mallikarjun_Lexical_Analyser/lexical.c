#include <ctype.h>
#include <string.h>
#include "lexical.h"

/* keywords that are data types */
static char *res_kwords_data[] = {
    "const", "volatile", "extern", "auto", "register", "static", "signed",
    "unsigned", "short", "long", "double", "char", "int", "float",
    "struct", "union", "enum", "void", "typedef", ""       
};

/* keywords that are not data types */
static char *res_kwords_non_data[] = {
    "goto", "return", "continue", "break", "if", "else", "for", "while",
    "do", "switch", "case", "default", "sizeof", ""
};
/* operators with two characters */
static char *two_char_ops[] = {
    "==", "!=", "<=", ">=", "++", "--", "+=", "-=", "*=", "/=", "%=",
    "&&", "||", "<<", ">>", "&=", "|=", "^=", "->", ""
};

static const char operators[] = "/+*-%=<>~&,!^|";
static const char symbols[]   = "(){}[]:;";

/* token type names used while printing */
static const char *type_names[] = {
    [KEYWORD_DATA]     = "KEYWORD_DATA",
    [KEYWORD_NON_DATA] = "KEYWORD_NON_DATA",
    [IDENTIFIER]       = "IDENTIFIER",
    [NUMERIC_CONST]    = "NUMERIC_CONST",
    [FLOAT_CONST]      = "FLOAT_CONST",
    [STRING]           = "STRING",
    [CHAR_CONST]       = "CHAR_CONST",
    [OPERATOR]         = "OPERATOR",
    [SYMBOL]           = "SYMBOL",
    [HEADER_FILE]      = "HEADER_FILE"
};
void lexer_run(lexer *lx)
{
    int ch, next;

    /* read the file character by character */
    while ((ch = fgetc(lx->fd)) != EOF) {
        /* newline, increase line number */
        if(ch == '\n')
        {
            lx->line++;
        }
        else if(isspace(ch))
        {
            continue;
        }
        /* word: keyword or identifier */
        else if(isalpha(ch) || ch == '_')
        {
            scan_word(lx,ch);
        }
        /* number */
        else if(isdigit(ch))
        {
            scan_number(lx, ch);
        }
        /* string or char constant */
        else if(ch == '"' || ch == '\'')
        {
             scan_quoted(lx, ch);
        }
        /* header file line starting with # */
        else if(ch == '#')
        {
            lx->word_idx = 0;
            lx->word[lx->word_idx++]= '#';
            while((next = fgetc(lx->fd)) != EOF && next != '\n')
            {
                 if (lx->word_idx < WORD_BUFF_SIZE - 1)
                 lx->word[lx->word_idx++] = next;
            }
            lx->word[lx->word_idx] = '\0';
            emit(lx, HEADER_FILE, lx->word);
            if (next == '\n')
                lx->line++;
        }
        /* '/' can be a comment or an operator */
        else if(ch == '/')
        {
            next = fgetc(lx->fd);
            if(next != EOF)
            {
                ungetc(next,lx->fd);
            }
            if(next == '/' || next == '*' )
            {
                scan_comment(lx);
            }
            else 
            {
                scan_operator(lx, ch);
            }
        }
         else if (ch != '\0' && strchr(operators, ch) != NULL) 
        {
            scan_operator(lx, ch);
        }
        /* symbols, also count the brackets */
        else if(ch != '\0' && strchr(symbols, ch)!= NULL )
        {
            int *count = NULL;
            int flag = 0;
            char sym[2];
            switch(ch)
            {
                case '{':count = &lx->brace_count;
                flag =1;
                break;
                 case '}':count = &lx->brace_count;
                flag =-1;
                break;
                 case '[':count = &lx->bracket_count;
                flag =1;
                break;
                 case ']':count = &lx->bracket_count;
                flag =-1;
                break;
                 case '(':count = &lx->paren_count;
                flag =1;
                break;
                 case ')':count = &lx->paren_count;
                flag =-1;
                break;
                default:
                break;
            }
           if(count != NULL)
            {
                *count += flag;
                /* closing bracket came without opening one */
                if(*count < 0)
                {
                    printf(RED"line %d : error: unmatched '%c'\n"RESET, lx->line, ch);
                    lx->error_cnt++;
                    *count = 0;
                }
            }
            sym[0] = (char)ch;
            sym[1] = '\0';
            emit(lx, SYMBOL, sym);
        }
 
        /* nothing matched */
        else {
            printf(RED"line %d : error: unknown character '%c'\n"RESET, lx->line, ch);
            lx->error_cnt++;
        }
    }
 
    /* end-of-file checks: anything still open was never closed */
    if (lx->brace_count > 0) {
        printf(RED"line %d : error: unmatched '{'\n"RESET, lx->line);
        lx->error_cnt++;
    }
    if (lx->bracket_count > 0) {
        printf(RED"line %d : error: unmatched '['\n"RESET, lx->line);
        lx->error_cnt++;
    }
    if (lx->paren_count > 0) {
        printf(RED"line %d : error: unmatched '('\n"RESET, lx->line);
        lx->error_cnt++;
    }
    /* print total errors at the end */
    printf(RED"\nTotal errors: %d\n"RESET, lx->error_cnt);
}
void scan_word(lexer *lx, int first)
{
    lx->word_idx = 0;
    lx->word[lx->word_idx++] = (char)first;
    int new;
    /* read letters, digits and underscore */
    while((new = fgetc(lx->fd))!=EOF && (isalnum(new) || new == '_'))
    {
        if(lx->word_idx < WORD_BUFF_SIZE - 1)
        lx->word[lx->word_idx++] = (char)new;
    }
    if(new != EOF)
    {
        ungetc(new,lx->fd);
    }
    lx->word[lx->word_idx] = '\0';
    /* check keyword table, else it is an identifier */
    if(lookup(res_kwords_data,lx->word))
     emit(lx, KEYWORD_DATA ,lx->word);
     else if(lookup(res_kwords_non_data,lx->word))
     emit(lx,KEYWORD_NON_DATA,lx->word);
     else
     emit(lx, IDENTIFIER, lx->word);
}
 
void scan_number(lexer *lx, int first)
{
    int ch,dots=0,valid=1;
   char *w = lx->word;
   w[0]=(char)first;
   lx->word_idx = 1;
    /* read the full number text */
    while((ch = fgetc(lx->fd)) != EOF && (ch == '.' || isalnum(ch)))
    {
         if(lx->word_idx < WORD_BUFF_SIZE - 1)
         {
            w[lx->word_idx++] = ch;
         }
        
    }
    if(ch != EOF)
    {
         ungetc(ch,lx->fd);
    }
    w[lx->word_idx]='\0';
    /*validate hexa decimal*/
    if(w[0]=='0' && (w[1]== 'x' || w[1] == 'X'))
    {
        valid = (lx->word_idx > 2);
        for(int i=2;w[i]!='\0';i++)
        {
            if(!(isxdigit(w[i])))
            {
                valid=0;
            }
        }
    }
    else  /*octal,decimal,float*/
    {
        /* count dots, any non digit is invalid */
        for(int i=0;w[i]!='\0';i++)
        {
            if(w[i] == '.')
            {
                dots++;
            }
            else if (!isdigit(w[i]))
                valid = 0;
        }
        if (dots > 1)
            valid = 0;
        if (w[0] == '0' && w[1] != '\0' && dots == 0)             /* octal: 0-7 only */
        {
            for (int i = 1; w[i] != '\0'; i++)
            {
                    if (w[i] > '7')
                        valid = 0;
            }
        }
    }
     if (valid)
        emit(lx, dots ? FLOAT_CONST : NUMERIC_CONST, w);
    else {
        printf(RED"line %d : error: invalid number '%s'\n"RESET, lx->line, w);
        lx->error_cnt++;
    }
}
void scan_quoted(lexer *lx, int quote)
{
    int   ch = 0, i = 0, closed = 0;
    char *w = lx->word;
 
    w[i++] = (char)quote;                            /* keep the opening quote */
 
    while (i < WORD_BUFF_SIZE - 2 && (ch = fgetc(lx->fd)) != EOF && ch != '\n') {
        w[i++] = (char)ch;
        if (ch == quote) 
        {                           /* closing quote: done */
            closed = 1;
            break;
        }
        if (ch == '\\') 
        {                            /* escape: keep the next char too */
            ch = fgetc(lx->fd);
            if (ch == EOF || ch == '\n')
            {
                break;
            }
            w[i++] = (char)ch;
        }
    }
    w[i] = '\0';
 
    if (ch == '\n')
        ungetc(ch, lx->fd);                          /* let lexer_run count the line */
 
    /* a string just needs its closing quote; a char constant is 'A' (3 chars) or '\n' (4) */
    if (closed && (quote == '"' || i == 3 || (i == 4 && w[1] == '\\')))
        emit(lx, quote == '"' ? STRING : CHAR_CONST, w);
    else 
    {
        printf(RED"line %d : error: bad %s: %s\n", lx->line,quote == '"' ? "string" : "char constant"RESET, w);
        lx->error_cnt++;
    }
}
void scan_comment(lexer *lx)
{
    int start=lx->line,ch,prev=0;
    ch = fgetc(lx->fd);
    if(ch == '/') /*single line comment*/
    {
        while((ch=fgetc(lx->fd))!= EOF && ch != '\n');
        if(ch == '\n')
        ungetc(ch,lx->fd);
    }
    else  /*multiline comment*/
    {
         while((ch = fgetc(lx->fd))!= EOF)
         {
            if(ch == '\n')
            {
                lx->line++;
            }
            /* end of comment found */
            if(prev == '*' && ch == '/')
            return;
            prev = ch;
         }
         /* reached EOF without closing */
         printf(RED"line %d : error: unterminated comment\n"RESET, start);
        lx->error_cnt++;
    }
  
}
void scan_operator(lexer *lx, int first)
{
    int next;
 
    lx->word[0] = (char)first;
    lx->word[1] = '\0';
 
    next = fgetc(lx->fd);                       /* peek at the next character */
    if (next != EOF) 
    {
        lx->word[1] = (char)next;
        lx->word[2] = '\0';
        if (!lookup(two_char_ops, lx->word)) {  /* not a two-character operator */
            lx->word[1] = '\0';                 /* keep only the first character */
            ungetc(next, lx->fd);               /* and give the second one back */
        }
    }
 
    emit(lx, OPERATOR, lx->word);
}

/* print the token with line number */
void emit(lexer *lx, token_type type, const char *text)
{
    printf("line %d : %s : %s\n", lx->line, type_names[type], text);
}

/* search word in the table, 1 if found */
int lookup(char *table[], const char *word)
{
    for(int i=0;table[i][0];i++)
    {
        if(strcmp(table[i],word)==0)
        {
            return 1;
        }
    }
    return 0;
}