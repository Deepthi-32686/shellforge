#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "executor.h"

/*
 * SIGCHLD handler
 *
 * Reap finished background children so that they do not
 * remain as zombie processes.
 */
static void sigchld_handler(int sig)
{
    int saved_errno = 0;
    (void)sig;

    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
        /*
         * Keep reaping children until there are no more
         * finished children waiting to be collected.
         */
    }

    (void)saved_errno;
}

/*
 * Install the SIGCHLD handler.
 */
static void setup_sigchld_handler(void)
{
    struct sigaction sa;

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    sigaction(SIGCHLD, &sa, NULL);
}

/*
 * Apply input/output redirection for a command.
 */
static int setup_redirection(command_t *cmd)
{
    int fd;

    if (cmd == NULL)
    {
        return 0;
    }

    /*
     * Input redirection:
     *
     * command < file
     */
    if (cmd->input[0] != '\0')
    {
        fd = open(cmd->input, O_RDONLY);

        if (fd < 0)
        {
            perror("open");
            return 0;
        }

        if (dup2(fd, STDIN_FILENO) < 0)
        {
            perror("dup2");
            close(fd);
            return 0;
        }

        close(fd);
    }

    /*
     * Output redirection:
     *
     * command > file
     */
    if (cmd->output[0] != '\0')
    {
        if (cmd->append)
        {
            fd = open(
                cmd->output,
                O_WRONLY | O_CREAT | O_APPEND,
                0644
            );
        }
        else
        {
            fd = open(
                cmd->output,
                O_WRONLY | O_CREAT | O_TRUNC,
                0644
            );
        }

        if (fd < 0)
        {
            perror("open");
            return 0;
        }

        if (dup2(fd, STDOUT_FILENO) < 0)
        {
            perror("dup2");
            close(fd);
            return 0;
        }

        close(fd);
    }

    return 1;
}

int execute_external(command_t *cmd)
{
    pid_t pid;
    int status;

    if (cmd == NULL || cmd->argc == 0 || cmd->argv[0] == NULL)
    {
        return 1;
    }

    /*
     * Make sure the SIGCHLD handler is installed.
     */
    setup_sigchld_handler();

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    /*
     * Child process
     */
    if (pid == 0)
    {
        /*
         * Background commands should not read from the
         * shell's terminal.
         *
         * Redirect stdin to /dev/null.
         */
        if (cmd->background)
        {
            int devnull = open("/dev/null", O_RDONLY);

            if (devnull < 0)
            {
                perror("open /dev/null");
                exit(1);
            }

            if (dup2(devnull, STDIN_FILENO) < 0)
            {
                perror("dup2");
                close(devnull);
                exit(1);
            }

            close(devnull);
        }

        /*
         * Apply normal I/O redirection if specified.
         */
        if (!setup_redirection(cmd))
        {
            exit(1);
        }

        execvp(cmd->argv[0], cmd->argv);

        perror("execvp");
        exit(1);
    }

    /*
     * Background command:
     *
     * Do NOT wait.
     */
    if (cmd->background)
    {
        printf("[Background PID %d]\n", pid);
        return 0;
    }

    /*
     * Foreground command:
     *
     * Wait for the child to finish.
     */
    if (waitpid(pid, &status, 0) < 0)
    {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status))
    {
        return 128 + WTERMSIG(status);
    }

    return 1;
}

int execute_pipeline(pipeline_t *pipeline)
{
    int command_count;
    int pipe_count;
    int pipefds[2 * (MAX_COMMANDS - 1)];
    pid_t pids[MAX_COMMANDS];
    int background;

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
     * A single command does not need pipeline handling.
     */
    if (command_count == 1)
    {
        return execute_external(&pipeline->commands[0]);
    }

    /*
     * The last command's background flag determines
     * whether the complete pipeline runs in background.
     */
    background =
        pipeline->commands[command_count - 1].background;

    pipe_count = command_count - 1;

    /*
     * Create all required pipes.
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
     * Create one child for each command.
     */
    for (int i = 0; i < command_count; i++)
    {
        pids[i] = fork();

        if (pids[i] < 0)
        {
            perror("fork");

            for (int j = 0; j < pipe_count; j++)
            {
                close(pipefds[j * 2]);
                close(pipefds[j * 2 + 1]);
            }

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
             * If this is not the first command,
             * read stdin from the previous pipe.
             */
            if (i > 0)
            {
                if (dup2(
                        pipefds[(i - 1) * 2],
                        STDIN_FILENO
                    ) < 0)
                {
                    perror("dup2");
                    exit(1);
                }
            }

            /*
             * If this is not the last command,
             * write stdout into the next pipe.
             */
            if (i < command_count - 1)
            {
                if (dup2(
                        pipefds[i * 2 + 1],
                        STDOUT_FILENO
                    ) < 0)
                {
                    perror("dup2");
                    exit(1);
                }
            }

            /*
             * Background pipeline:
             *
             * stdin of the first command is redirected
             * to /dev/null.
             */
            if (background && i == 0)
            {
                int devnull = open("/dev/null", O_RDONLY);

                if (devnull < 0)
                {
                    perror("open /dev/null");
                    exit(1);
                }

                if (dup2(devnull, STDIN_FILENO) < 0)
                {
                    perror("dup2");
                    close(devnull);
                    exit(1);
                }

                close(devnull);
            }

            /*
             * Close every original pipe descriptor after
             * dup2().
             */
            for (int j = 0; j < pipe_count; j++)
            {
                close(pipefds[j * 2]);
                close(pipefds[j * 2 + 1]);
            }

            /*
             * Apply explicit redirection.
             */
            if (!setup_redirection(
                    &pipeline->commands[i]
                ))
            {
                exit(1);
            }

            /*
             * Execute the command.
             */
            execvp(
                pipeline->commands[i].argv[0],
                pipeline->commands[i].argv
            );

            perror("execvp");
            exit(1);
        }
    }

    /*
     * Parent closes all pipe descriptors.
     */
    for (int i = 0; i < pipe_count; i++)
    {
        close(pipefds[i * 2]);
        close(pipefds[i * 2 + 1]);
    }

    /*
     * Background pipeline:
     *
     * Do not wait for the children.
     * Print the PID of the first process.
     */
    if (background)
    {
        printf("[Background PID %d]\n", pids[0]);
        return 0;
    }

    /*
     * Foreground pipeline:
     *
     * Wait for every process.
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
