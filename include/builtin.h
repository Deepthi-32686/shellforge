#ifndef BUILTIN_H
#define BUILTIN_H

int execute_builtin(int argc, char **argv);

int builtin_cd(int argc, char **argv);
int builtin_pwd(int argc, char **argv);
int builtin_echo(int argc, char **argv);
int builtin_exit(int argc, char **argv);

#endif
