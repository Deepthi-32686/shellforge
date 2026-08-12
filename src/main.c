#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "lexer.h"

const char *token_type_name(TokenType type)
{
    switch (type)
    {
        case TOKEN_WORD:
            return "WORD";

        case TOKEN_PIPE:
            return "PIPE";

        case TOKEN_REDIRECT_IN:
            return "REDIRECT_IN";

        case TOKEN_REDIRECT_OUT:
            return "REDIRECT_OUT";

        case TOKEN_APPEND:
            return "APPEND";

        case TOKEN_END:
            return "END";

        default:
            return "UNKNOWN";
    }
}

int main(void)
{
    printf("=====================================\n");
    printf("Shellforge\n");
    printf("Tokenizer and Lexer - Milestone 2\n");
    printf("=====================================\n");

    while (1)
    {
        char *line = readline("shellforge$ ");

        if (line == NULL)
        {
            printf("\nGoodbye!\n");
            break;
        }

        if (strlen(line) == 0)
        {
            free(line);
            continue;
        }

        add_history(line);

        /* History command */
        if (strcmp(line, "history") == 0)
        {
            HIST_ENTRY **hist = history_list();

            if (hist != NULL)
            {
                for (int i = 0; hist[i] != NULL; i++)
                {
                    printf("%d  %s\n", i + 1, hist[i]->line);
                }
            }

            free(line);
            continue;
        }

        /* Exit command */
        if (strcmp(line, "exit") == 0)
        {
            free(line);
            printf("Exiting...\n");
            break;
        }

        int count = 0;

        Token *tokens = tokenize(line, &count);

        if (tokens == NULL)
        {
            fprintf(stderr, "Error: Tokenization failed.\n");
            free(line);
            continue;
        }

        for (int i = 0; i < count; i++)
        {
            printf("Token: %-15s Value: %s\n",
                   token_type_name(tokens[i].type),
                   tokens[i].value);
        }

        free_tokens(tokens, count);
        free(line);
    }

    return 0;
}
