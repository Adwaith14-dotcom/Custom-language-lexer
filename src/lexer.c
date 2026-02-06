#include "lexer.h"
#include "token.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

void init_lexer(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->length = strlen(source);
    lexer->current = 0;
    lexer->line = 1;
    lexer->column = 1;
}

static int is_at_end(Lexer* lexer) {
    return lexer->current >= lexer->length;
}

static char peek(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->source[lexer->current];
}

static char peek_next(Lexer* lexer) {
    if (lexer->current + 1 >= lexer->length) return '\0';
    return lexer->source[lexer->current + 1];
}

static char advance(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';

    char c = lexer->source[lexer->current++];
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    return c;
}

static void skip_whitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch(c) {
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                advance(lexer);
                break;
            default:
                return;
        }
    }
}

static int match(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return 0;
    if (lexer->source[lexer->current] != expected) return 0;

    lexer->current++;
    lexer->column++;
    return 1;
}

static Token* scan_identifier(Lexer* lexer) {
    size_t start = lexer->current;
    int start_col = lexer->column;

    while (isalpha(peek(lexer)) || isdigit(peek(lexer)) || peek(lexer) == '_') {
        advance(lexer);
    }

    size_t len = lexer->current - start;
    char* lexeme = (char*) malloc(len + 1);
    memcpy(lexeme, &lexer->source[start], len);
    lexeme[len] = '\0';

    TokenType type = TOKEN_IDENTIFIER;

    if (strcmp(lexeme, "if") == 0) type = TOKEN_IF;
    else if (strcmp(lexeme, "else") == 0) type = TOKEN_ELSE;
    else if (strcmp(lexeme, "while") == 0) type = TOKEN_WHILE;
    else if (strcmp(lexeme, "for") == 0) type = TOKEN_FOR;
    else if (strcmp(lexeme, "return") == 0) type = TOKEN_RETURN;
    else if (strcmp(lexeme, "int") == 0) type = TOKEN_INT;
    else if (strcmp(lexeme, "float") == 0) type = TOKEN_FLOAT;
    else if (strcmp(lexeme, "char") == 0) type = TOKEN_CHAR;

    Token* token = create_token(type, lexeme, lexer->line, start_col);
    free(lexeme); 
    return token;
}

static Token* scan_number(Lexer* lexer) {
    size_t start = lexer->current;
    int start_col = lexer->column;

    while (isdigit(peek(lexer))) advance(lexer);

    if (peek(lexer) == '.' && isdigit(peek_next(lexer))) {
        advance(lexer); 
        while (isdigit(peek(lexer))) advance(lexer);
    }

    size_t len = lexer->current - start;
    char* lexeme = (char*) malloc(len + 1);
    memcpy(lexeme, &lexer->source[start], len);
    lexeme[len] = '\0';
 
    TokenType type = strchr(lexeme, '.') ? TOKEN_FLOAT_LITERAL : TOKEN_INT_LITERAL;

    Token* token = create_token(type, lexeme, lexer->line, start_col);
    free(lexeme);
    return token;
}

static Token* scan_string(Lexer* lexer) {
    int start_col = lexer->column;
    advance(lexer); 

    size_t start = lexer->current;

    while (peek(lexer) != '"' && !is_at_end(lexer)) {
        if (peek(lexer) == '\n') lexer->line++; 
        advance(lexer);
    }

    if (is_at_end(lexer)) {
        return create_token(TOKEN_UNKNOWN, "", lexer->line, start_col);
    }

    size_t len = lexer->current - start;
    char* lexeme = (char*) malloc(len + 1);
    memcpy(lexeme, &lexer->source[start], len);
    lexeme[len] = '\0';

    advance(lexer); 

    Token* token = create_token(TOKEN_STRING_LITERAL, lexeme, lexer->line, start_col);
    free(lexeme);
    return token;
}

Token* get_next_token(Lexer* lexer) {
    skip_whitespace(lexer);

    if (is_at_end(lexer)) return create_token(TOKEN_EOF, "", lexer->line, lexer->column);

    char c = advance(lexer);

    int start_col = lexer->column - 1;

    switch(c) {
        case '+': return create_token(TOKEN_PLUS, "+", lexer->line, start_col);
        case '-': return create_token(TOKEN_MINUS, "-", lexer->line, start_col);
        case '*': return create_token(TOKEN_STAR, "*", lexer->line, start_col);
        case '/': return create_token(TOKEN_SLASH, "/", lexer->line, start_col);
        case '(': return create_token(TOKEN_LPAREN, "(", lexer->line, start_col);
        case ')': return create_token(TOKEN_RPAREN, ")", lexer->line, start_col);
        case '{': return create_token(TOKEN_LBRACE, "{", lexer->line, start_col);
        case '}': return create_token(TOKEN_RBRACE, "}", lexer->line, start_col);
        case ';': return create_token(TOKEN_SEMICOLON, ";", lexer->line, start_col);
        case ',': return create_token(TOKEN_COMMA, ",", lexer->line, start_col);
    }

    if (c == '=' && match(lexer, '=')) return create_token(TOKEN_EQUAL, "==", lexer->line, start_col);
    if (c == '!' && match(lexer, '=')) return create_token(TOKEN_NOT_EQUAL, "!=", lexer->line, start_col);
    if (c == '<') {
        if (match(lexer, '=')) return create_token(TOKEN_LESS_EQUAL, "<=", lexer->line, start_col);
        return create_token(TOKEN_LESS, "<", lexer->line, start_col);
    }
    if (c == '>') {
        if (match(lexer, '=')) return create_token(TOKEN_GREATER_EQUAL, ">=", lexer->line, start_col);
        return create_token(TOKEN_GREATER, ">", lexer->line, start_col);
    }
    if (c == '=') return create_token(TOKEN_ASSIGN, "=", lexer->line, start_col);

    if (isalpha(c) || c == '_') return scan_identifier(lexer);

    if (isdigit(c)) return scan_number(lexer);

    if (c == '"') return scan_string(lexer);

    char unk[2] = {c, '\0'};
    return create_token(TOKEN_UNKNOWN, unk, lexer->line, start_col);
}
