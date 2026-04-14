#include"terminal.h"
#include"home.h"
#include<stdlib.h>

void run_app() {
    int choice;
    do {
        MessageProp prop;
        render_home_screen(&prop);
        choice = cursor_scroll(prop.choice_cnt, NEW_LINE_ENABLED);
        process_choice(choice, prop.choice_cnt);
    } 
    while (choice != -1);
} 

int main() { 
    run_app();
    canonical_mode();
    return 0;
}