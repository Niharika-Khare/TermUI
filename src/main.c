#include"terminal.h"
#include"home.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

void run_app() {
    int choice;
    do {
        MessageProp prop;
        render_home_screen(&prop);
        choice = cursor_scroll(prop.choice_cnt, NEW_LINE_ENABLED);
        choice = process_choice(choice, prop.choice_cnt);
    } 
    while (choice != -1);
} 

void exit_app() {
    clear_terminal();
    if (canonical_mode() == -1) {
        printf("Exiting TermUI application abnormally...\n");
    } else {
        printf("Exiting TermUI application...\n");
    }    
}

int main() { 
    run_app();
    atexit(exit_app);
    return 0;
}