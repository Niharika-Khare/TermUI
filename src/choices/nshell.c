#include "nshell.h"
#include "terminal.h"

static const char nshell_prompt[] = BOLD_TEXT_ON \
                                    "\n[%s]\nnshell: " \
                                    BOLD_TEXT_OFF;

static const char *supported_tools[] = { "&&", "|", "||" };

static const char *io_redirectives[] = { "<", ">", ">>", "2>", "&>" };

static const char *unsupported_tools[] = { ";", "#", "&", "$", "\\" };

static char path[PATH_MAX];
static int stdin_org_fd;
static int stdout_org_fd;
static int stderr_org_fd;

static inline void log_shell_err(const char *err_msg, ...) {
    char err_buff[500];
    va_list args;
    va_start(args, err_msg);
    int bytes = vsnprintf(err_buff, sizeof(err_buff), err_msg, args);
    va_end(args);
    write(STDERR_FILENO, err_buff, bytes);
}

static inline void log_shell_info(const char *info_msg, ...) {
    char info_buff[500];
    va_list args;
    va_start(args, info_msg);
    int bytes = vsnprintf(info_buff, sizeof(info_buff), info_msg, args);
    va_end(args);
    write(STDOUT_FILENO, info_buff, bytes);
}

static inline void setup_file_decriptors() {
    stdin_org_fd = dup(STDIN_FILENO);
    stdout_org_fd = dup(STDOUT_FILENO);
    stderr_org_fd = dup(STDERR_FILENO);
}

static inline void reset_file_descriptors() {
    dup2(stdin_org_fd, STDIN_FILENO);
    dup2(stdout_org_fd, STDOUT_FILENO);
    dup2(stderr_org_fd, STDERR_FILENO);
}

static inline void close_file_descriptors() {
    close(stdin_org_fd);
    close(stdout_org_fd);
    close(stderr_org_fd);
}

static inline void setup_nshell_terminal() {
    setup_file_decriptors();
    clear_terminal();
    canonical_mode();
    getcwd(path, PATH_MAX);
}

static inline void print_prompt() {
    char buffer[PATH_MAX + sizeof(nshell_prompt) + 1];
    int bytes = snprintf(buffer, PATH_MAX + sizeof(nshell_prompt) + 1, nshell_prompt, path);
    write(STDOUT_FILENO, buffer, bytes);           
}                        

static inline int read_cmd(char* buffer) {
    int bytes = read(STDIN_FILENO, buffer, MAX_READ_BUFFER - 1);
    if (bytes > 0) {
        buffer[bytes] = '\0';
    }
    return bytes;
}

static inline int is_io_redirective(char * token) {
    int redirective_cnt = sizeof(io_redirectives)/sizeof(io_redirectives[0]);
    for (int i=0; i<redirective_cnt; i++) {
        if (strcmp(token, io_redirectives[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static inline int is_supported_tool(char * token) {
    int tool_cnt = sizeof(supported_tools)/sizeof(supported_tools[0]);
    for (int i=0; i<tool_cnt; i++) {
        if (strcmp(token, supported_tools[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static inline int is_unsupported_tool(char * token) {
    int tool_cnt = sizeof(unsupported_tools)/sizeof(unsupported_tools[0]);
    for (int i=0; i<tool_cnt; i++) {
        if (strcmp(token, unsupported_tools[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static void inline save_history() {
    log_shell_info("\n\n[ Saving history... ]\n");
    // Save history
    log_shell_info("[ History saved... ]\n");
    log_shell_info("[ Exiting nshell... ]\n");
}

static inline int apply_io_redirect(Command *command) {
    if (command->io_rd_cnt > 0) {
        int fd;
        for (int i=0; i<command->io_rd_cnt; i++) {
            if (strcmp(command->io_rd[i].redirect, "<") == 0) {
                fd = open(command->io_rd[i].filename, O_RDONLY);
                if (fd == -1) {
                    log_shell_err("err: file not found: %s\n", command->io_rd[i].filename);
                    return -1;
                }
                dup2(fd, STDIN_FILENO);
            } 
            else if (strcmp(command->io_rd[i].redirect, ">") == 0) {
                fd = open(command->io_rd[i].filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    log_shell_err("err: file not found: %s\n", command->io_rd[i].filename);
                    return -1;
                }
                dup2(fd, STDOUT_FILENO);
            }
            else if (strcmp(command->io_rd[i].redirect, "2>") == 0) {
                fd = open(command->io_rd[i].filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    log_shell_err("err: file not found: %s\n", command->io_rd[i].filename);
                    return -1;
                }
                dup2(fd, STDERR_FILENO);
            }
            else if (strcmp(command->io_rd[i].redirect, ">>") == 0) {
                fd = open(command->io_rd[i].filename, O_WRONLY | O_APPEND | O_CREAT , 0644);
                if (fd == -1) {
                    log_shell_err("err: file not found: %s\n", command->io_rd[i].filename);
                    return -1;
                }
                dup2(fd, STDOUT_FILENO);
            }
            else if (strcmp(command->io_rd[i].redirect, "&>") == 0) {
                fd = open(command->io_rd[i].filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    log_shell_err("err: file not found: %s\n", command->io_rd[i].filename);
                    return -1;
                }
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
            }
            if (fd != -1) {
                close(fd);
            }
        }
    }
    return 0;
}

/*
 * Tokenizes the raw input buffer and builds a linked Command tree rooted at root_command. 
 * Each whitespace-delimited token is classified as: 
 *  1. A command name/ command argument, 
 *  2. An IO redirect operator (<, >, >>, 2>, &>)
 *  3. An IO redirect filename (expected immediately after a redirect via the wait_on_fname flag)
 *  4. A pipeline/logical connector (&&, ||, |) — which allocates the next Command node in the chain.
 * 
 * Returns 1 on success, 0 on any parse error (unsupported operators (both io redirect and pipeline),
 * missing filename after redirect, token overflow, dangling connector, etc.). Errors
 * are written directly to stderr via log_shell_err.
 */
static int parse_commands(char *buffer, Command * root_command) {
    
    Command *command = root_command;
    int param_cnt = 0, io_rd_cnt = 0, wait_on_fname = 0;

    while (*buffer != '\n' && *buffer != '\0') {
        
        while (*buffer == ' ' || *buffer == '\t') {
            buffer++;
        }

        char token[MAX_TOKEN_LEN];
        int token_len=0;

        while (token_len < MAX_TOKEN_LEN
            && *buffer != ' ' 
            && *buffer != '\t'
            && *buffer != '\n' 
            && *buffer != '\0') {
                token[token_len++] = *buffer++;
        }

        if (token_len == MAX_TOKEN_LEN) {
            log_shell_err("err: word length should be less than: %d\n", MAX_TOKEN_LEN);
            return 0;
        }

        if (token_len) {

            token[token_len] = '\0';
            
            if (wait_on_fname) {

                if (!*command->cmd || is_supported_tool(token)) {
                    log_shell_err("err: parse error near: %s\n", token);
                    return 0;
                }
                if (is_unsupported_tool(token)) {
                    log_shell_err("err: parse error near: %s\n%s not supported yet\n", token, token);
                    return 0;
                }
                if (is_io_redirective(token)) {
                    log_shell_err("err: missing filename for io redirection\n");
                    return 0;
                }
                if (io_rd_cnt >= MAX_IO_REDIRECTS) {
                    log_shell_err("err: too many io redirects!!\n");
                    return 0;
                }

                memcpy(command->io_rd[io_rd_cnt].filename, token, token_len+1);
                command->io_rd_cnt = ++io_rd_cnt;
                wait_on_fname = 0;
            }
            else if (is_io_redirective(token)) {

                if (!*command->cmd) {
                    log_shell_err("err: parse error near: %s\n", token);
                    return 0;
                }
                if (io_rd_cnt >= MAX_IO_REDIRECTS) {
                    log_shell_err("err: too many io redirects!!\n");
                    return 0;
                }

                memcpy(command->io_rd[io_rd_cnt].redirect, token, token_len+1);
                wait_on_fname = 1;
            }
            else if (is_supported_tool(token)) {

                if (!*command->cmd) {
                    log_shell_err("err: parse error near: %s\n", token);
                    return 0;
                }

                param_cnt = 0;
                io_rd_cnt = 0;
                memcpy(command->tool, token, token_len+1);
                command->next_cmd = malloc(sizeof(Command));
                memset(command->next_cmd, 0, sizeof(Command));
                command = command->next_cmd;
            } 
            else if (is_unsupported_tool(token)) {

                log_shell_err("err: nshell does not support this shell tool yet: %s\n", token);
                return 0;
            }
            else if (!*command->cmd) {

                command->param_cnt = param_cnt + 1;
                memcpy(command->cmd_params[param_cnt++], token, token_len+1);
                memcpy(command->cmd, token, token_len+1);
            }  
            else if (param_cnt < MAX_TOKEN_COUNT-1) {

                command->param_cnt = param_cnt + 1;
                memcpy(command->cmd_params[param_cnt++], token, token_len+1);
            }
            else if (param_cnt >= MAX_TOKEN_COUNT-1) {

                log_shell_err("err: too many command parameters, should be less than: %d\n", MAX_TOKEN_COUNT);
                return 0;
            }
        } 
    }
    if ((!*command->cmd && command != root_command) || wait_on_fname) {
        log_shell_err("err: incorrect termination to command sequence\n");
        return 0;
    } 
    return 1;
}

/**
 * Execute the commands sequentially after parsing
 */
static int execute_commands(Command *command) {
    int success = 1;
    while (command) {
        if (*command->cmd) {
            success = 1;
            if (memcmp(command->cmd, BI_EXIT, sizeof(BI_EXIT)) == 0) {
                return 1;
            }
            else if (memcmp(command->cmd, BI_CD, sizeof(BI_CD)) == 0) {
                if (apply_io_redirect(command) == -1) {
                    return 0;
                }
                if (command->param_cnt < 2 || chdir(command->cmd_params[1]) == -1) {
                    log_shell_err("err: cd: invalid path: %s\n", command->cmd_params[1]);
                    success = 0;
                } 
                getcwd(path, PATH_MAX);
                reset_file_descriptors();
            }
            else if (memcmp(command->cmd, BI_PWD, sizeof(BI_PWD)) == 0) {
                if (apply_io_redirect(command) == -1) {
                    return 0;
                }
                char path[PATH_MAX];
                if (getcwd(path, PATH_MAX) == NULL) {
                    log_shell_err("err: pwd: unable to get current working directory\n");
                    success = 0;
                }
                else {
                    char buffer[PATH_MAX+2];
                    int bytes = snprintf(buffer, PATH_MAX + 2, "%s\n", path);
                    write(STDOUT_FILENO, buffer,  bytes);
                }
                reset_file_descriptors();
            }
            else if (memcmp(command->cmd, BI_ECHO, sizeof(BI_ECHO)) == 0) {
                if (apply_io_redirect(command) == -1) {
                    return 0;
                }
                char buffer[2048];
                int bytes = 0;
                for (int i=1; i<command->param_cnt && bytes < 2048; i++) {
                    bytes += snprintf(buffer + bytes, strlen(command->cmd_params[i]) + 2, "%s ", command->cmd_params[i]);
                }
                buffer[bytes] = '\n';
                write(STDOUT_FILENO, buffer, bytes + 1);
                reset_file_descriptors();
            }
            else {
                pid_t cmd_pid = fork();
                if (cmd_pid < 0) {
                    log_shell_err("err: encountered error in execution of: %s\n", command->cmd);
                    break;
                } else if (cmd_pid == 0) {
                    if (apply_io_redirect(command) == -1) {
                        return 0;
                    }
                    char path[PATH_MAX] = CMD_PATH;
                    strcat(path, command->cmd);

                    char *args[MAX_TOKEN_COUNT];
                    for (int i=0; i<command->param_cnt; i++) {
                        args[i] =  command->cmd_params[i];
                    }
                    args[command->param_cnt] = NULL;

                    if (execv(path, args) < 0) {
                        log_shell_err("err: command not found: %s\n", command->cmd);
                        exit(1);
                    }
                } else {
                    int status;
                    if (wait(&status) != -1) {
                        success = WEXITSTATUS(status) == 0;
                    }
                }
            }
            if (*command->tool) {
                if (memcmp(command->tool, "||", sizeof("||")) == 0 && success) {
                    return 0;
                } else if (memcmp(command->tool, "&&", sizeof("&&")) == 0 && !success) {
                    return 0;
                } else if (memcmp(command->tool, "|", sizeof("|")) == 0) {
                
                }
            } 
            command = command->next_cmd;
        } else {
            break;  
        }
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
        free(command);
    }
}

/**
 * 
 * Entry point for the command interpreter nshell
 * 
 * This interpreter check if the command's executable exists at the path ./bin/commands. 
 * If it exists, the command is executable is loaded and executed as a child process, 
 * otherwise command not found error is thrown.
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
            Command root_command;
            memset(&root_command, 0, sizeof(Command));

            int parsing_status = parse_commands(buffer, &root_command);

            if (parsing_status) {
                exit_status = execute_commands(&root_command);
            }

            clear_space(root_command.next_cmd);
            reset_file_descriptors();
            // add to history buffer
        }
    } while (!exit_status);

    close_file_descriptors();
    save_history();
    
    getc(stdin);
    return 0;
}