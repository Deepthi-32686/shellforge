#include <stdio.h>
#include <string.h>

#include "token.h"

void token_init(token_list_t *list)
{
    list->count = 0;

    for (int i = 0; i < MAX_TOKENS; i++)
    {
        list->tokens[i].type = TOKEN_END;
        list->tokens[i].text[0] = '\0';
    }
}

void token_print(const token_list_t *list)
{
    printf("\n========== TOKENS ==========\n");

    for (int i = 0; i < list->count; i++)
    {
        printf("Token %d: ", i);

        switch (list->tokens[i].type)
        {
            case TOKEN_WORD:
                printf("WORD");
                break;

            case TOKEN_INPUT:
                printf("INPUT");
                break;

            case TOKEN_OUTPUT:
                printf("OUTPUT");
                break;

            case TOKEN_APPEND:
                printf("APPEND");
                break;

            case TOKEN_PIPE:
                printf("PIPE");
                break;

            case TOKEN_BACKGROUND:
                printf("BACKGROUND");
                break;

            case TOKEN_END:
                printf("END");
                break;
        }

        printf(" -> %s\n", list->tokens[i].text);
    }

    printf("============================\n");
}
