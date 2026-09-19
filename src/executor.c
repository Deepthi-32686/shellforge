#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "executor.h"

int execute_external(command_t *cmd)
{
    pid_t pid;
    int status;

    if (cmd == NULL || cmd->argc == 0 || cmd->argv[0] == NULL)
    {
        return 1;
    }

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        execvp(cmd->argv[0], cmd->argv);

        perror("execvp");
        exit(1);
    }

    if (waitpid(pid, &status, 0) < 0)
    {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }

    return 1;
}

int execute_pipeline(pipeline_t *pipeline)
{
    int command_count;
    int pipe_count;
    int pipefds[2 * (MAX_COMMANDS - 1)];
    pid_t pids[MAX_COMMANDS];

    if (pipeline == NULL)
    {
        return 1;
    }

    command_count = pipeline->command_count;

    if (command_count <= 0)
    {
        return 1;
    }

    /*
     * A single command does not need a pipe.
     */
    if (command_count == 1)
    {
        return execute_external(&pipeline->commands[0]);
    }

    pipe_count = command_count - 1;

    /*
     * Create all required pipes.
     *
     * For:
     *
     *     ls | grep src | wc -l
     *
     * We need 2 pipes:
     *
     *     ls ---- pipe 0 ----> grep ---- pipe 1 ----> wc
     */
    for (int i = 0; i < pipe_count; i++)
    {
        if (pipe(&pipefds[i * 2]) < 0)
        {
            perror("pipe");

            for (int j = 0; j < i; j++)
            {
                close(pipefds[j * 2]);
                close(pipefds[j * 2 + 1]);
            }

            return 1;
        }
    }

    /*
     * Create one child process for each command.
     */
    for (int i = 0; i < command_count; i++)
    {
        pids[i] = fork();

        if (pids[i] < 0)
        {
            perror("fork");

            /*
             * Close all pipe file descriptors in the parent.
             */
            for (int j = 0; j < pipe_count; j++)
            {
                close(pipefds[j * 2]);
                close(pipefds[j * 2 + 1]);
            }

            /*
             * Wait for children that were already created.
             */
            for (int j = 0; j < i; j++)
            {
                waitpid(pids[j], NULL, 0);
            }

            return 1;
        }

        /*
         * Child process
         */
        if (pids[i] == 0)
        {
            /*
             * If this is NOT the first command,
             * connect stdin to the previous pipe.
             *
             * Example:
             *
             *     ls | grep src
             *
             * grep must read from pipe 0.
             */
            if (i > 0)
            {
                if (dup2(pipefds[(i - 1) * 2], STDIN_FILENO) < 0)
                {
                    perror("dup2");
                    exit(1);
                }
            }

            /*
             * If this is NOT the last command,
             * connect stdout to the next pipe.
             *
             * Example:
             *
             *     ls | grep src
             *
             * ls must write into pipe 0.
             */
            if (i < command_count - 1)
            {
                if (dup2(pipefds[i * 2 + 1], STDOUT_FILENO) < 0)
                {
                    perror("dup2");
                    exit(1);
                }
            }

            /*
             * The child no longer needs any of the original
             * pipe file descriptors after dup2().
             *
             * Close ALL pipe descriptors.
             */
            for (int j = 0; j < pipe_count; j++)
            {
                close(pipefds[j * 2]);
                close(pipefds[j * 2 + 1]);
            }

            /*
             * Replace the child process with the external command.
             */
            execvp(
                pipeline->commands[i].argv[0],
                pipeline->commands[i].argv
            );

            /*
             * execvp() returns only if an error occurred.
             */
            perror("execvp");
            exit(1);
        }
    }

    /*
     * Parent process no longer needs any pipe descriptors.
     */
    for (int i = 0; i < pipe_count; i++)
    {
        close(pipefds[i * 2]);
        close(pipefds[i * 2 + 1]);
    }

    /*
     * Wait for every child process.
     *
     * The exit status of the LAST command in the pipeline
     * becomes the pipeline's exit status.
     */
    int last_status = 1;

    for (int i = 0; i < command_count; i++)
    {
        int status;

        if (waitpid(pids[i], &status, 0) < 0)
        {
            perror("waitpid");
            continue;
        }

        if (i == command_count - 1)
        {
            if (WIFEXITED(status))
            {
                last_status = WEXITSTATUS(status);
            }
            else if (WIFSIGNALED(status))
            {
                last_status = 128 + WTERMSIG(status);
            }
        }
    }

    return last_status;
}
