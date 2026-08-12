#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"

static char *copy_string(const char *start, size_t length)
{
    char *result = malloc(length + 1);

    if (result == NULL)
    {
        return NULL;
    }

    memcpy(result, start, length);
    result[length] = '\0';

    return result;
}

Token *tokenize(const char *input, int *count)
{
    Token *tokens = malloc(sizeof(Token) * MAX_TOKENS);

    if (tokens == NULL)
    {
        return NULL;
    }

    *count = 0;

    size_t i = 0;

    while (input[i] != '\0' && *count < MAX_TOKENS - 1)
    {
        while (isspace((unsigned char)input[i]))
        {
            i++;
        }

        if (input[i] == '\0')
        {
            break;
        }

        TokenType type;
        size_t start = i;
        size_t length;

        if (input[i] == '|')
        {
            type = TOKEN_PIPE;
            i++;
            length = 1;
        }
        else if (input[i] == '<')
        {
            type = TOKEN_REDIRECT_IN;
            i++;
            length = 1;
        }
        else if (input[i] == '>')
        {
            if (input[i + 1] == '>')
            {
                type = TOKEN_APPEND;
                i += 2;
                length = 2;
            }
            else
            {
                type = TOKEN_REDIRECT_OUT;
                i++;
                length = 1;
            }
        }
        else
        {
            type = TOKEN_WORD;

            while (input[i] != '\0' &&
                   !isspace((unsigned char)input[i]) &&
                   input[i] != '|' &&
                   input[i] != '<' &&
                   input[i] != '>')
            {
                i++;
            }

            length = i - start;
        }

        tokens[*count].type = type;
        tokens[*count].value = copy_string(input + start, length);

        if (tokens[*count].value == NULL)
        {
            free_tokens(tokens, *count);
            return NULL;
        }

        (*count)++;
    }

    tokens[*count].type = TOKEN_END;
    tokens[*count].value = NULL;

    return tokens;
}
