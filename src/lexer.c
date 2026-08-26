#include <ctype.h>
#include <string.h>

#include "lexer.h"

static void add_token(token_list_t *list, token_type_t type,
                      const char *text)
{
    if (list->count >= MAX_TOKENS - 1)
    {
        return;
    }

    list->tokens[list->count].type = type;

    strncpy(list->tokens[list->count].text,
            text,
            MAX_TOKEN_LEN - 1);

    list->tokens[list->count].text[MAX_TOKEN_LEN - 1] = '\0';

    list->count++;
}

void lexer(const char *input, token_list_t *list)
{
    token_init(list);

    int i = 0;

    while (input[i] != '\0' &&
           list->count < MAX_TOKENS - 1)
    {
        while (isspace((unsigned char)input[i]))
        {
            i++;
        }

        if (input[i] == '\0')
        {
            break;
        }

        if (input[i] == '|')
        {
            add_token(list, TOKEN_PIPE, "|");
            i++;
        }
        else if (input[i] == '<')
        {
            add_token(list, TOKEN_INPUT, "<");
            i++;
        }
        else if (input[i] == '>')
        {
            if (input[i + 1] == '>')
            {
                add_token(list, TOKEN_APPEND, ">>");
                i += 2;
            }
            else
            {
                add_token(list, TOKEN_OUTPUT, ">");
                i++;
            }
        }
        else if (input[i] == '&')
        {
            add_token(list, TOKEN_BACKGROUND, "&");
            i++;
        }
        else
        {
            char word[MAX_TOKEN_LEN];
            int j = 0;

            while (input[i] != '\0' &&
                   !isspace((unsigned char)input[i]) &&
                   input[i] != '|' &&
                   input[i] != '<' &&
                   input[i] != '>' &&
                   input[i] != '&')
            {
                if (j < MAX_TOKEN_LEN - 1)
                {
                    word[j++] = input[i];
                }

                i++;
            }

            word[j] = '\0';

            add_token(list, TOKEN_WORD, word);
        }
    }

    list->tokens[list->count].type = TOKEN_END;
    list->tokens[list->count].text[0] = '\0';
}
