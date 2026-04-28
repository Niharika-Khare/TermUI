#ifndef _COMMON_
#define _COMMON_

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

void log_err(char * err_msg, ...) {

    char err_buffer[2048];
    va_list args;
    va_start(args, err_msg);
    int bytes = vsnprintf(err_buffer, sizeof(err_buffer), err_msg, args);
    write(STDERR_FILENO, err_buffer, bytes);
    va_end(args);
}

#endif /** _COMMON_ */