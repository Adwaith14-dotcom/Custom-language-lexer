#include "token.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Token* create_token(TokenType type, const char* lexeme, int line, int column) {
    Token* token = (Token*) malloc(sizeof(Token));
    if (!token) {
        fprintf(stderr, "Memory allocation failed for token\n");
        return NULL;
    }

    size_t len = strlen(lexeme);
    token->lexeme = (char*) malloc(len + 1);
    if (!token->lexeme) {
        fprintf(stderr, "Memory allocation failed for lexeme\n");
        free(token);
        return NULL;
    }

    memcpy(token->lexeme, lexeme, len);
    token->lexeme[len] = '\0';

    token->type = type;
    token->line = line;
    token->column = column;

    return token;
}

void destroy_token(Token* token) {
    if (!token) return;
    if (token->lexeme) free(token->lexeme);
    free(token);
}
