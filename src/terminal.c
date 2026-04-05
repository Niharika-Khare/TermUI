#include"../include/terminal.h"
#include<unistd.h>
#include<stdio.h>

typedef struct termios Terminal;

Terminal original_term = {0};

/**
 * Move cursor to the specified coordinates
 */
void relocate_cursor(int x, int y) {
    printf("\033[%d;%dH", x, y);
}

/**
 * Clear the terminal and move the cursor to the top left corner.
 */
void clear_terminal() {
    char* clear_terminal = CLEAR_SCREEN CURSOR_HOME;
    write(STDOUT_FILENO, clear_terminal, sizeof(clear_terminal));
}

/**
 * Revert the Terminal back to original settings.
 * 
 * termios.c_cc[VMIN] = 1 and termios.c_cc[VTIME] = 0 are default settings.
*/
void canonical_mode() {
    if ((original_term.c_lflag & ICANON) && (original_term.c_lflag & ECHO)) {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
    } else {
        Terminal t;
        tcgetattr(STDIN_FILENO, &t);
        t.c_lflag = t.c_lflag | ICANON | ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
    }
}

/**
 * This method will set the terminal to non-canonical mode if it is in canonical mode.
 * It will save the current terminal settings to the global variable "original_term"
 * 
 */
void non_canonical_mode() {
    Terminal t, new_term;
    tcgetattr(STDIN_FILENO, &t);
    if ((t.c_lflag & ICANON) && (t.c_lflag & ECHO)) {
        original_term = t;
        new_term = t;
        new_term.c_lflag = new_term.c_lflag & ~ICANON & ~ECHO;
        new_term.c_cc[VMIN] = 1;
        new_term.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
    }
}

void cursor_movement(int c) {

}