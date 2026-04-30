#ifndef _COMMON_
#define _COMMON_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>


static inline void log_err(char * err_msg, ...) {

    char err_buffer[2048];
    va_list args;
    va_start(args, err_msg);
    int bytes = vsnprintf(err_buffer, sizeof(err_buffer), err_msg, args);
    write(STDERR_FILENO, err_buffer, bytes);
    va_end(args);
}

static inline void log_info(char * msg, ...) {

    char buffer[2048];
    va_list args;
    va_start(args, msg);
    int bytes = vsnprintf(buffer, sizeof(buffer), msg, args);
    write(STDERR_FILENO, buffer, bytes);
    va_end(args);
}


static inline char* itoa(int i) {
    char *str = malloc(sizeof (int) + 1);
    snprintf(str, sizeof(int) + 1, "%d", i);
    return str;
}

#endif /** _COMMON_ */