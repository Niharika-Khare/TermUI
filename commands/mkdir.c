#include "mkdir.h"
#include "common.h"

static int flags;
static int mode = DEFAULT_MODE;
static int exit_status = 0;

int parse_params(char ** argv, char ** dir_list) {
    while (*++argv) {
        if (strcmp(*argv, "-m") == 0) {
            flags = flags | M_;
            if (*++argv == NULL) {
                log_err("err: mode value missing!\n");
                exit_status = 1;
                return -1;
            }
            if (atoi(*argv) > 0777) {
                log_err("err: inccorrect mode value %s!\n", *argv);
                exit_status = 1;
                return -1;
            }
            mode = atoi(*argv);
        }
        else if (strcmp(*argv, "-v") == 0) {
            flags = flags | V_;
        } else {
            *dir_list++ = *argv;
        }
    }
    *dir_list = NULL;
    return 0;
}

int main(int argc, char ** argv) {

    char *dir_list[MAX_DIR_COUNT];
    char buffer[2048];
    int bytes = 0;
    
    if (parse_params(argv, dir_list) == -1) {
        return exit_status;
    }

    for (int i=0; dir_list[i]; i++) {
        if (mkdir(dir_list[i], mode) == -1) {
            log_err("mkdir: unable to create: %s\n", dir_list[i]);
            exit_status = 1;
        } 
        else if (flags & V_) {
            bytes += snprintf(buffer + bytes, strlen(dir_list[i]) + 2, "%s\n", dir_list[i]);
        }
    }

    write(STDOUT_FILENO, buffer, bytes);

    return exit_status;
}