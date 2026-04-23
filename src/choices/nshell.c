#include"nshell.h"
#include"terminal.h"

static const char *valid_commands[] = { "cat", "cd", "copy", "echo", "exit", \
                                        "grep", "ls", "man", "mkdir", "move", \
                                        "pipe", "pwd", "rm" };

static const char *supported_tools[] = { "&&", "|", "||" };

static const char *io_redirectives[] = { "<", ">", ">>", "2>", "&>" };

static const char *unsupported_tools[] = { ";", "#", "&", "$", "\\" };

static inline void print_prompt() {
    write(STDOUT_FILENO, nshell_prompt, sizeof(nshell_prompt)-1);           
}                        


void log_shell_err(const char *err_msg, ...) {
    char err_buff[500];
    va_list args;
    va_start(args, err_msg);
    int bytes = vsnprintf(err_buff, sizeof(err_buff), err_msg, args);
    va_end(args);
    write(STDERR_FILENO, err_buff, bytes);
}

void log_shell_info(const char *info_msg, ...) {
    char err_buff[500];
    va_list args;
    va_start(args, info_msg);
    int bytes = vsnprintf(err_buff, sizeof(err_buff), info_msg, args);
    va_end(args);
    write(STDOUT_FILENO, err_buff, bytes);
}

static inline void setup_nshell_terminal() {
    clear_terminal();
    canonical_mode();
}

static inline int read_cmd(char* buffer) {
    int bytes = read(STDIN_FILENO, buffer, MAX_READ_BUFFER - 1);
    if (bytes > 0) {
        buffer[bytes] = '\0';
    }
    return bytes;
}

static int is_valid_command(char * token) {
    int cmd_cnt = sizeof(valid_commands)/sizeof(valid_commands[0]);
    for (int i=0; i<cmd_cnt; i++) {
        if (strcmp(token, valid_commands[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_io_redirective(char * token) {
    int redirective_cnt = sizeof(io_redirectives)/sizeof(io_redirectives[0]);
    for (int i=0; i<redirective_cnt; i++) {
        if (strcmp(token, io_redirectives[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_supported_tool(char * token) {
    int tool_cnt = sizeof(supported_tools)/sizeof(supported_tools[0]);
    for (int i=0; i<tool_cnt; i++) {
        if (strcmp(token, supported_tools[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_unsupported_tool(char * token) {
    int tool_cnt = sizeof(unsupported_tools)/sizeof(unsupported_tools[0]);
    for (int i=0; i<tool_cnt; i++) {
        if (strcmp(token, unsupported_tools[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int parse_commands(char *buffer, Command * command) {
    
    int param_cnt = 0, parsing_start=0;

    while (*buffer != '\n' && *buffer != '\0') {
        
        while (*buffer == ' ') {
            buffer++;
        }

        char token[MAX_TOKEN_LEN];
        int token_len=0;

        while (token_len < MAX_TOKEN_LEN
            && *buffer != ' ' 
            && *buffer != '\n' 
            && *buffer != '\0') {
                token[token_len++] = *buffer++;
        }

        if (token_len == MAX_TOKEN_LEN) {
            log_shell_err("Word length should be less than %d\n", MAX_TOKEN_LEN);
            break;
        }

        if (token_len) {

            parsing_start=1;
            token[token_len] = '\0';
            
            if (is_valid_command(token) && command->cmd == NULL) {
                command->cmd = malloc(token_len+1);
                memcpy(command->cmd, token, token_len+1);
            } 
            else if (is_supported_tool(token)) {

                if (!command->cmd && !param_cnt) {
                    log_shell_err("Parse error near %s\n", token);
                    break;
                }
                command->tool = malloc(token_len+1);
                memcpy(command->tool, token, token_len+1);
                command->parse_status = 1;
                command->next_cmd = malloc(sizeof(Command));
                command = command->next_cmd;
                param_cnt = 0;
            } 
            else if (param_cnt < MAX_TOKEN_COUNT) {
                command->cmd_params[param_cnt] = malloc(token_len+1);
                memcpy(command->cmd_params[param_cnt++], token, token_len+1);
                command->param_cnt = param_cnt;
            }

            if (param_cnt == MAX_TOKEN_COUNT) {
                log_shell_err("Too many command parameters, should be less than %d\n", MAX_TOKEN_COUNT);
                command->parse_status = 0;
                command->next_cmd = malloc(sizeof(Command));
                command = command->next_cmd;
                param_cnt = 0;
            }
        } 
    }
    if (!command->cmd && !param_cnt && parsing_start) {
        log_shell_err("Incorrect termination to command sequence\n");
        return 0;
    } 
    command->parse_status = parsing_start;
    return parsing_start;
}

/**
 * Excute the commands sequentially after parsing
 */
static int execute_commands(Command *command) {
    int success = 1;
    while (command) {
        if (command->cmd) {
            success = 1;
            if (memcmp(command->cmd, "exit", sizeof("exit")) == 0) {
                printf("Exiting\n");
                return 1;
            }
            pid_t cmd_pid = fork();
            if (cmd_pid < 0) {
                log_shell_err("Encountered error in execution of %s\n", command->cmd);
                break;
            } else if (cmd_pid == 0) {
                char path[50] = CMD_PATH;
                // TODO: Replace below with memcat for better performance
                strcat(path, command->cmd);
                if (execv(path, command->cmd_params) < 0) {
                    log_shell_err("Unable to find the command %s\n", command->cmd);

                    // TODO: If unsuccessful, send the status to parent (nshell)
                    // via pipe or signal.

                    exit(1);
                }
            } else {
                wait(NULL);
                // TODO: if child returns failed then 
                // success = 0;
            }
        } else {
            log_shell_err("Invalid command: %s!\n", command->cmd_params[0]);
            success = 0;
        }
        if (command->tool) {
            if (memcmp(command->tool, "||", sizeof("||")) == 0 && success) {
                return 0;
            } else if (memcmp(command->tool, "&&", sizeof("&&")) == 0 && !success) {
                return 0;
            } else if (memcmp(command->tool, "|", sizeof("|")) == 0) {
                
            }
        } 
        command = command->next_cmd;
    }
    return 0;
}

/**
 * Recursively clear all the memory allocated to the Command Tree during parsing.
 * Without this, program suffers from memory leaks.
 */
void clear_space(Command *command) {
    if (command != NULL) {
        clear_space(command->next_cmd);
        free(command->cmd);
        free(command->tool);
        for(int i = 0 ; i < command->param_cnt; i++) {
            free(command->cmd_params[i]);
        }
        free(command);
    }
}


/**
 * 
 * Entry point for the command interpreter nshell
 * 
 * This interpreter currently supports a bunch of commands which are listed in the 
 * valid_commands list. To add any new command, currently, this list need to be modified 
 * and executable need to be added in the ./bin/command/ folder of the TermUI directory. 
 * 
 * Scope of improvement: Move past the list and check if the command's executable exists 
 * in the path ./bin/commands. This way there won't be any need to change interpreter 
 * code whenever new nshell command is implemented.
 * 
 */
int nshell() {
    setup_nshell_terminal();
    int exit_status = 0;
    do {
        print_prompt();
        char buffer[MAX_READ_BUFFER];
        int bytes = read_cmd(buffer);

        if (bytes > 0) {
            Command *root_command;
            root_command = malloc(sizeof(Command));
            memset(root_command, 0, sizeof(Command));

            int parsing_status = parse_commands(buffer, root_command);

            if (parsing_status) {
                exit_status = execute_commands(root_command);
            }

            clear_space(root_command);

            // add to history buffer
        }
    } while (!exit_status);

    log_shell_info("\n\n[ Saving history... ]\n");
    // Save history
    log_shell_info("[ History saved... ]\n");
    log_shell_info("[ Exiting nshell... ]");
    getc(stdin);
    return 0;
}