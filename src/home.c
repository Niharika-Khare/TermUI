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
    char buffer[MAX_HOME_SCREEN_BUFFER];
    char header[] = DISABLE_LINE_WRAP \
                    "**********************************************************************\n"   \
                    "                WELCOME TO TERMINAL UI APPLICATION                    \n"   \
                    "**********************************************************************\n\n" \
                    "Select one of the choice_menu (Press q/Q to quit application):        \n"   \
                    ENABLE_LINE_WRAP ;
    int len = snprintf(buffer, sizeof(buffer), "%s", header);
    for (int i=0; i<choice_cnt; i++) {
        len += snprintf(buffer+len, sizeof(buffer), "%d. %s\n", i+1, choice_menu[i]);
    }
    write(STDOUT_FILENO, buffer, len);
    return get_header_line_count(header);
}

/**
 * Print the application header and the choices available to the user.
 * Switch to non-canonical mode to enable the user to navigate the choice menu.
 */
int render_home_screen(MessageProp * prop) {

    // Validate input
    if (prop == NULL) {
        char err_msg[] = "Invalid arguements for screen rendering!\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg)-1);
        return -1;
    }

    // Setup terminal
    clear_terminal();

    // Display header and the choice menu
    int choice_cnt = sizeof(choice_menu)/sizeof(choice_menu[0]);
    int header_len = display_choice_menu(choice_menu, choice_cnt);

    if (non_canonical_mode() == -1) {
        char err_msg[] = "Switch to non-canonical mode unsuccessful!\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg)-1);
        return -1;
    } 

    // Populate props
    prop->scroll_start = header_len + 1;
    prop->choice_cnt = choice_cnt;

    // Position the cursor for navigating the choice menu
    relocate_cursor(prop->scroll_start, 1);
    return 0;
}

/**
 * Select appropriate response for the user choice.
 * 
 * QUIT_CODE is returned by cursor_scroll.c_ver when user presses 'q' or 'Q'
 * ESCAPE_CODE is returned by cursor_scroll.c_ver when user presses ESC to go back to home screen
 */
int process_choice(int choice, int choice_cnt) {
    if (choice == ESCAPE_CODE) {
        return ESCAPE_CODE;
    } 
    else if (choice == QUIT_CODE) {
        return QUIT_CODE;
    } 
    else if (choice >=1 && choice <= choice_cnt) {
        return func[choice - 1]();
    } 
    return choice;
}