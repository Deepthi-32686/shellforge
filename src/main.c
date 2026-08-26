#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "expand.h"
#include "builtin.h"

int main(void)
{
    token_list_t tokens;
    pipeline_t pipeline;
    char *line;

    printf("========================================\n");
    printf("    Shellforge\n");
    printf(" A Unix Style Shell written in C\n");
    printf("========================================\n");

    while (1)
    {
        line = readline("shellforge$ ");

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

        lexer(line, &tokens);
        token_print(&tokens);

        if (parse(&tokens, &pipeline))
        {
            expand_variables(&pipeline);
            pipeline_print(&pipeline);

            /*
             * Milestone 3.1:
             * Execute built-in commands in the shell process.
             */
            if (pipeline.command_count == 1)
            {
                execute_builtin(
                    pipeline.commands[0].argc,
                    pipeline.commands[0].argv
                );
            }
        }

        free(line);
    }

    return 0;
}
