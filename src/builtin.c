#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "builtin.h"

int execute_builtin(int argc, char **argv)
{
    if (argc == 0 || argv == NULL || argv[0] == NULL)
    {
        return 0;
    }

    if (strcmp(argv[0], "cd") == 0)
    {
        return builtin_cd(argc, argv);
    }
    else if (strcmp(argv[0], "pwd") == 0)
    {
        return builtin_pwd(argc, argv);
    }
    else if (strcmp(argv[0], "echo") == 0)
    {
        return builtin_echo(argc, argv);
    }
    else if (strcmp(argv[0], "exit") == 0)
    {
        return builtin_exit(argc, argv);
    }

    return 0;
}

int builtin_cd(int argc, char **argv)
{
    char *dir;

    if (argc == 1)
    {
        dir = getenv("HOME");

        if (dir == NULL)
        {
            fprintf(stderr, "cd: HOME environment variable not set\n");
            return 1;
        }
    }
    else if (argc == 2)
    {
        dir = argv[1];
    }
    else
    {
        fprintf(stderr, "cd: too many arguments\n");
        return 1;
    }

    if (chdir(dir) != 0)
    {
        perror("cd");
        return 1;
    }

    return 1;
}

int builtin_pwd(int argc, char **argv)
{
    (void)argv;

    if (argc > 1)
    {
        fprintf(stderr, "pwd: too many arguments\n");
        return 1;
    }

    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("pwd");
        return 1;
    }

    printf("%s\n", cwd);

    return 1;
}

int builtin_echo(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        printf("%s", argv[i]);

        if (i < argc - 1)
        {
            printf(" ");
        }
    }

    printf("\n");

    return 1;
}

int builtin_exit(int argc, char **argv)
{
    (void)argv;

    if (argc > 1)
    {
        fprintf(stderr, "exit: too many arguments\n");
    }

    exit(0);
}
