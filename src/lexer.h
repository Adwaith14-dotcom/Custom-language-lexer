#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <stddef.h>

typedef struct {
    const char* source;
    size_t length;      
    size_t current;     
    int line;           
    int column;         
} Lexer;

void init_lexer(Lexer* lexer, const char* source);

Token* get_next_token(Lexer* lexer);

#endif 
