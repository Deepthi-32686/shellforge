#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

int execute_external(command_t *cmd);
int execute_pipeline(pipeline_t *pipeline);

#endif
