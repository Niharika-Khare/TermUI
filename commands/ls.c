#include "ls.h"
#include "common.h"

static inline void set_flag(int *flags, int flag) {
    *flags =  *flags | flag;
}

static char *buffer;
static int buf_len;
static int flags;
static int exit_status;


void ls(int flags, char *dir_name) {
    DIR* dir;
    Dirent * d;

    if ((dir = opendir(dir_name)) == NULL) {
        log_err("ls: unable to open: %s\n", dir_name);
        exit_status = 1;
        return;
    }
    
    buffer = realloc(buffer, buf_len + strlen(dir_name) + 4);
    buf_len += snprintf(buffer + buf_len, strlen(dir_name) + 4, "\n%s:\n", dir_name);

    char delim = '\t';
    char *dir_list[MAX_DIR_CNT];
    int i=0;

    while ((d = readdir(dir))) {
        if (!(flags & A_) && d->d_name[0] == '.') {
            continue;
        } 
        if ((flags & ONE_)) {
            delim = '\n';
        }
        if ( (flags & R_) 
            && (d->d_type & DT_DIR) 
            && strcmp(d->d_name, ".") != 0 
            && strcmp(d->d_name, "..") != 0) {

                int bytes = strlen(dir_name) + strlen("/") + strlen(d->d_name);
                dir_list[i] = malloc(bytes + 1);
                snprintf(dir_list[i++], bytes + 1, "%s/%s", dir_name, d->d_name);
        }

        int len = strlen(d->d_name);
        buffer = realloc(buffer, buf_len + len + 2);
        buf_len += snprintf(buffer + buf_len,len + 2, "%s%c", d->d_name, delim);
    }

    buffer[buf_len++] = '\n';
    dir_list[i] = NULL;

    for (i=0; dir_list[i]; i++) {
        ls(flags, dir_list[i]);
    }
    closedir(dir);

    for (i=0; dir_list[i]; i++) {
        free(dir_list[i]);
    }
    return;
}

void parse_params(char ** argv, char ** dir_list) {
    while(*++argv) {
        if (strcmp(*argv, "-a") == 0) {
            set_flag(&flags, A_);
        } 
        else if (strcmp(*argv, "-R") == 0) {
            set_flag(&flags, R_);
        }
        else if (strcmp(*argv, "-1") == 0) {
            set_flag(&flags, ONE_);
        }
        else {
            *dir_list++ = *argv;
        }
    }
    *dir_list = NULL;
}


int main(int argc, char ** argv) {

    char *dir_list[MAX_DIR_CNT];
    parse_params(argv, dir_list);
    
    if (!*dir_list) {
        *dir_list = ".";
        *(dir_list + 1) = NULL;
    }
    buffer = malloc(0);
    for (int i=0; dir_list[i]; i++) {
        ls(flags, dir_list[i]);
    }

    write(STDOUT_FILENO, buffer, buf_len);
    free(buffer);

    return exit_status;
}