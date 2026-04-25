#include "pwd.h"

int pwd() {
    DIR *dir;
    if ((dir = opendir(CWD)) == NULL) {
        return -1;
    }

    char path[512];
    if (fcntl(dir->__dd_fd, F_GETPATH, &path) == -1) {        
        return -1;
    }
    int bytes = strlen(path);
    path[bytes] = '\n';
    path[bytes+1] = '\0';
    write(STDOUT_FILENO, path, strlen(path) + 2);
    return 0;
}

int main() {
    if (pwd() == -1) {
        char err_msg[512] = "err: unable to get path of current working directory!\n";
        write (STDERR_FILENO, err_msg, strlen(err_msg)+1);
        // signal failure to parent (nshell)
        return 1;
    }
    return 0;
}