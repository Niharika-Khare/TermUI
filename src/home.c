#include"explorer.h"
#include"help.h"
#include"history.h"
#include"home.h"
#include"nshell.h"
#include"sysinfo.h"
#include"terminal.h"
#include<unistd.h>
#include<stdio.h>

static char *choice_menu[] = {SYS_INFO, EXPLR, N_SHELL, HISTORY, HELP};

/**
 * Array of pointers to functions for handling the user choice from choice_menu
 */
static int (*func[])() = {system_info, explorer, nshell, history, help};

static int get_header_line_count(char * header) {
    int num_line = 0;
    while (*header != '\0') {
        num_line += (*header++ == NEW_LINE);
    }
    return num_line;
}

static int display_choice_menu(char **choice_menu, int choice_cnt) {
    clear_terminal();
    char header[] = DISABLE_LINE_WRAP \
                    "**********************************************************************\n" \
                    "                WELCOME TO TERMINAL UI APPLICATION                    \n" \
                    "**********************************************************************\n\n" \
                    "Select one of the choice_menu (Press q/Q to quit application):        \n" \
                    ENABLE_LINE_WRAP ;
    printf("%s", header);
    for (int i=0; i<choice_cnt; i++) {
        printf("%d. %s\n", i+1, choice_menu[i]);
    }
    return get_header_line_count(header);
}

/**
 * Print the application header and the choices available to the user.
 * Switch to non-cacnonical mode to enable the user to navigate the choice menu.
 */
void render_home_screen(MessageProp * prop) {
    if (prop == NULL) {
        return;
    }

    int choice_cnt = sizeof(choice_menu)/sizeof(choice_menu[0]);
    int header_len = display_choice_menu(choice_menu, choice_cnt);

    prop->scroll_start = header_len + 1;
    prop->choice_cnt = choice_cnt;

    non_canonical_mode();
    relocate_cursor(prop->scroll_start, 0);
}

/**
 * Select appropriate response for the user choice.
 * 
 * -1 is returned by cursor_scroll when user presses 'q' or 'Q'
 * 0 is returned by cursor_scroll when user presses ESC to go back to home screen
 */
int process_choice(int choice, int choice_cnt) {
    if (choice == -1) {
        return -1;
    } 
    else if (choice == 0) {
        return 0;
    } 
    else if (choice <= choice_cnt) {
        return func[choice-1]();
    } 
    else {
        char err_msg[] = "Not a valid option!\n";
        write(STDOUT_FILENO, err_msg, sizeof(err_msg));
        return 0;
    }
}