#include "sysinfo.h"
#include "terminal.h"
#include <stdio.h>

static void display_info() {
    OsInfo os_info;
    uname(&os_info);
    printf("%s\n", os_info.sysname);
    printf("%s\n", os_info.nodename);
    printf("%s\n", os_info.machine);
    printf("%s\n", os_info.release);
    printf("%s\n", os_info.version);

}

int system_info() {
    Winsize w;
    get_window_size(&w);
    clear_terminal();
    display_info();
    relocate_cursor(0,0);
    POS cursor_pos = cursor_scroll(w.ws_row, HORIZONTAL_NAV);
    return cursor_pos.c_vert;
}