#include "common.h"
#include "copy.h"

static int exit_status = 0;

int parse_params(int *flags, char ** argv, char ** fname_list, char ** dest_file, char ** dest_dir) {

    int f_cnt = 0;
    while (*++argv) {
        if (strcmp(*argv, "-r") == 0 || strcmp(*argv, "-R") == 0) {
            *flags = *flags | R_;
        } 
        else if (strcmp(*argv, "-v") == 0) {
            *flags = *flags | V_;
        }
        else {
            *fname_list++ = *argv;
            f_cnt++;
        }
    }
    if (f_cnt < 2) {
        log_err("err: copy: destination missing!\n");
        exit_status = 1;
        return -1;
    } 
    else if (f_cnt == 2) {
        *dest_file = malloc(strlen(*--fname_list) + 1);
        strcpy(*dest_file, *fname_list);
        *fname_list = NULL;
    }
    else {
        *dest_dir = malloc(strlen(*--fname_list) + 1);
        strcpy(*dest_dir, *fname_list);
        *fname_list = NULL;
    } 
    return 0;
}

int main(int argc, char ** argv) {
    int flags = 0;
    char *fname_list[MAX_FILE_COUNT];
    char *dest_file = NULL;
    char *dest_dir = NULL;

    if (parse_params(&flags, argv, fname_list, &dest_file, &dest_dir) == -1) {
        return exit_status;
    }

    char buffer[MAX_BUFFER_SIZE];
    if (dest_file) {
        int s_fd = open(*fname_list, O_RDONLY);
        if (s_fd == -1) {
            log_err("err: copy: unable to open file: %s\n", *fname_list);
            exit_status = 1;
            return exit_status;
        }

        int d_fd = open(dest_file, O_WRONLY| O_TRUNC | O_CREAT, DEFAULT_MODE);
        if (d_fd == -1) {
            log_err("err: copy: unable to open or create file: %s\n", *dest_file);
            close(s_fd);
            exit_status = 1;
            return exit_status;
        }

        int bytes = 0;
        while ((bytes = read(s_fd, buffer, sizeof(buffer)))) {
            write(d_fd, buffer, bytes);
        }
        close(s_fd);
        close(d_fd);
    }
    else {
        for (int i=0; fname_list[i]; i++) {
            int s_fd = open(fname_list[i], O_RDONLY);
            if (s_fd == -1) {
                log_err("err: copy: unable to open file: %s\n", fname_list[i]);
                exit_status = 1;
                continue;
            }

            char dest_full_path[PATH_MAX];
            snprintf(dest_full_path, strlen(dest_dir) + strlen(fname_list[i]) + 2, "%s/%s", dest_dir, fname_list[i]);

            int d_fd = open(dest_full_path, O_WRONLY | O_TRUNC | O_CREAT, DEFAULT_MODE);
            if (d_fd == -1) {
                log_err("err: copy: unable to open or create file: %s\n", dest_full_path);
                close(s_fd);
                exit_status = 1;
                continue;
            }

            int bytes = 0;
            while ((bytes = read(s_fd, buffer, sizeof(buffer)))) {
                write(d_fd, buffer, bytes);
            }
            close(s_fd);
            close(d_fd);
        }
    }

    dest_dir ? free(dest_dir): dest_dir;
    dest_file ? free(dest_file): dest_file;
    return exit_status;
}