#ifndef _N_SHELL_
#define _N_SHELL_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_READ_BUFFER         2000
#define MAX_TOKEN_LEN           200
#define MAX_TOKEN_COUNT         50
#define BOLD_TEXT_ON            "\033[1m"
#define BOLD_TEXT_OFF           "\033[0m"
#define CMD_PATH                "./bin/commands/"

typedef struct command_struct {
    char *cmd;
    char *tool;
    int param_cnt;
    char *cmd_params[MAX_TOKEN_COUNT];
    int parse_status;
    struct command_struct * next_cmd;
} Command;
                            

void log_shell_err(const char *err_msg, ...);
void log_shell_info(const char *err_msg, ...);
void clear_space(Command *command);
int nshell();

#endif /* _N_SHELL_ */