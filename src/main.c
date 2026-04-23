#include"terminal.h"
#include"home.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdarg.h>

void run_app() {
    int choice = -1;
    do {
        MessageProp prop;
        if (!render_home_screen(&prop)) {
            choice = cursor_scroll(prop.choice_cnt, NEW_LINE_ENABLED);
            choice = process_choice(choice, prop.choice_cnt);
        } 
    } 
    while (choice != -1);
} 

void exit_app() {
    clear_terminal();
    if (canonical_mode() == -1) {
        char err_msg[] = "Exiting TermUI application abnormally...\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg)-1);
    } else {
        char msg[] = "Exiting TermUI application...\n";
        write(STDOUT_FILENO, msg, sizeof(msg)-1);
    }    
}

int main(int argc, char **argv) { 
    run_app();
    exit_app();
    return 0;
}