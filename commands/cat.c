#include "common.h"
#include "cat.h"

static void parse_params(int *flags, char ** argv, char ** f_list) {
    while (*++argv) {
        if (strcmp(*argv, "-b") == 0) {
            *flags = *flags | B_;
        }
        else if (strcmp(*argv, "-n") == 0) {
            *flags = *flags | N_;
        } 
        else {
            *f_list++ = *argv;
        } 
    }
    *f_list = NULL;
}

int main(int argc, char ** argv) {

    int flags, exit_status = 0;
    char *f_list[MAX_FILE_COUNT];

    parse_params(&flags, argv, f_list);

    for (int i=0; f_list[i]; i++) {
        int fd = open(f_list[i], O_RDONLY);

        if (fd == -1) {
            log_err("err: cat: unable of open file: %s\n", f_list[i]);
            exit_status = 1;
            continue;
        }
        char buffer[MAX_BUFFER_SIZE];
        int bytes = 0;
        while ((bytes = read(fd, buffer, sizeof (buffer)))) {
            write(STDOUT_FILENO, buffer, bytes);
        }

        close(fd);
    }
    return exit_status;
}