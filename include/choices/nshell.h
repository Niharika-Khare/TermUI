#ifndef _N_SHELL_
#define _N_SHELL_

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/wait.h>


/** Limits */
#define MAX_READ_BUFFER             2000
#define MAX_TOKEN_LEN               200
#define MAX_TOKEN_COUNT             50
#define MAX_IO_REDIRECTS            10


/** ANSI sequences */
#define BOLD_TEXT_ON                "\033[1m"
#define BOLD_TEXT_OFF               "\033[0m"


/** Path macros */
#define CMD_PATH                    "/Users/niharikakhare/POCs/TermUI/bin/commands/"


/** Shell built-ins */
#define BI_EXIT                     "exit"
#define BI_CD                       "cd"
#define BI_PWD                      "pwd"
#define BI_ECHO                     "echo"


typedef struct io_redirect {
    char redirect[MAX_TOKEN_LEN];
    char filename[MAX_TOKEN_LEN];
} IO_RD;


typedef struct command_struct {
    char cmd[MAX_TOKEN_LEN];
    char tool[MAX_TOKEN_LEN];
    int param_cnt;
    char cmd_params[MAX_TOKEN_COUNT][MAX_TOKEN_LEN];
    int io_rd_cnt;
    IO_RD io_rd[MAX_IO_REDIRECTS];
    struct command_struct * next_cmd;
} Command;
                    


void clear_space(Command *command);
int nshell();


#endif /* _N_SHELL_ */