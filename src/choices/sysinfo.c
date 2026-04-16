#include"sysinfo.h"
#include"terminal.h"
#include<stdio.h>

int system_info() {
    clear_terminal();
    printf("SystemInfo\n");
    return cursor_scroll(1000, HORIZONTAL_NAV);
}