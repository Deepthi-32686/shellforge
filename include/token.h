#ifndef TOKEN_H
#define TOKEN_H

#define MAX_TOKENS 100

typedef enum {
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIRECT_IN,
    TOKEN_REDIRECT_OUT,
    TOKEN_APPEND,
    TOKEN_END
} TokenType;

typedef struct {
    TokenType type;
    char *value;
} Token;

void free_tokens(Token *tokens, int count);

#endif
