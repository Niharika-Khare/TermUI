#include "help.h"
#include "terminal.h"

static void display_info() {
    
}

int help() {
    Winsize w;
    get_window_size(&w);
    clear_terminal();
    display_info();
    relocate_cursor(0,0);
    POS cursor_pos = cursor_scroll(w.ws_row, HORIZONTAL_NAV);
    return cursor_pos.c_vert;
}