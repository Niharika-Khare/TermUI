#include "history.h"
#include "home.h"
#include "terminal.h"
#include <stdio.h>
#include <unistd.h>


void init_app() {
    init_history();
}

void run_app() {
    int choice = -1;
    do {
        MessageProp prop;
        if (!render_home_screen(&prop)) {
            POS start = {1, 1};
            POS cursor_pos = cursor_scroll(start, prop.choice_cnt, ENTER_ENABLED);
            choice = cursor_pos.c_vert;
            choice = process_choice(choice, prop.choice_cnt);
        } 
    } 
    while (choice != QUIT_CODE);
} 

void exit_app() {
    clear_terminal();
    
    if (canonical_mode() == -1) {
        char err_msg[] = "Exiting TermUI application abnormally...\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg)-1);
    } 
    else {
        char msg[] = "Exiting TermUI application...\n";
        write(STDOUT_FILENO, msg, sizeof(msg)-1);
    }    
}

int main(int argc, char **argv) { 
    init_app();
    run_app();
    exit_app();
    return 0;
}