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
#include "executor.h"

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
             * Single command:
             *
             * First check whether it is a built-in.
             * If it is not a built-in, execute it as
             * an external command.
             */
            if (pipeline.command_count == 1)
            {
                command_t *cmd = &pipeline.commands[0];

                if (!execute_builtin(cmd->argc, cmd->argv))
                {
                    execute_external(cmd);
                }
            }
            /*
             * Multiple commands:
             *
             * Execute them as a pipeline.
             */
            else
            {
                execute_pipeline(&pipeline);
            }
        }

        free(line);
    }

    return 0;
}
