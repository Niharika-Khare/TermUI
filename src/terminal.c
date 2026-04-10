#include"terminal.h"
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct termios Terminal;

static Terminal original_term;
static int is_canon = 1;

static int get_terminal(Terminal *t, int file_no) {
    int tc;
    if ((tc = tcgetattr(file_no, t)) == -1) {
        char err_msg[] = "Failed to get terminal attributes!";
        write(file_no, err_msg, sizeof(err_msg));
    } 
    return tc;
}

static int set_terminal(Terminal *t, int file_no) {
    int tc;
    if ((tc = tcsetattr(file_no, TCSANOW, t)) == -1) {
        char err_msg[] = "Failed to set terminal attributes!";
        write(file_no, err_msg, sizeof(err_msg));
    }
    return tc;
}

/**
 * Move cursor to the specified coordinates
 * 
 * CHANGE THIS TO USE SNPRINTF() + WRITE()
 */
void relocate_cursor(int x, int y) {
    char cursor_loc[15];
    int len = snprintf(cursor_loc, sizeof(cursor_loc), "\033[%d;%dH", x, y);
    write(STDOUT_FILENO, cursor_loc, len);
}

/**
 * Move the cursor to the top left corner and then clear from cursor pos to the 
 * end of screen. 
 * 
 * Other options:
 *  1. CURSOR_HOME CLEAR_SCREEN  ->  scrollback buffer would be visible
 *  2. CURSOR_HOME CLEAR_SCREEN CLEAR_SCROLLBACK -> same behavior as the current 
 *                             impl (scrollback buffer would alse be earsed) but 
 *                             \033[3J is non-standard hennce less portable
 */
void clear_terminal() {
    char clear_terminal_cmd[] = CURSOR_HOME CURSOR_TO_END_CLEAR;
    write(STDOUT_FILENO, clear_terminal_cmd, sizeof(clear_terminal_cmd));
}

/**
 * Set the terminal to canonical mode either by reverting the non-canonical
 * settings or by explicitly setting the canonical flags.
 * 
 * termios.c_cc[VMIN] = 1 and termios.c_cc[VTIME] = 0 are default settings.
*/
int canonical_mode() {
    if ((original_term.c_lflag & ICANON) && (original_term.c_lflag & ECHO)) {
        return set_terminal(&original_term, STDIN_FILENO);
    } else {
        Terminal t;
        if (get_terminal(&t, STDIN_FILENO)) {
            return -1;
        }
        t.c_lflag |= ICANON | ECHO;
        t.c_cc[VMIN] = 1;
        t.c_cc[VTIME] = 0;
        return set_terminal(&t, STDIN_FILENO);
    }
}

/**
 * This method will set the terminal to non-canonical mode if it is in canonical mode.
 * It will save the current terminal settings to the global variable "original_term"
 * 
 */
int non_canonical_mode() { 
    Terminal t, new_term;
    if (get_terminal(&t, STDIN_FILENO)) {
        return -1;
    }
    if (!(t.c_lflag & ICANON) && !(t.c_lflag & ECHO)) {
        return 0;
    }
    original_term = t;
    t.c_lflag &= ~ICANON & ~ECHO;
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    return set_terminal(&t, STDIN_FILENO);
}


/**
 * Based on the content length (scroll_len), move the cursor to navigate any list
 */
int cursor_scroll(int scroll_len, int enable_side_move) {
    char buf[10];
    int cursor=0;
    while (1) {
        int bytes = read(STDIN_FILENO, buf, 10);
        if (bytes==1) {
            if (buf[0] == ENTER) {
                return cursor+1;
            } else if (buf[0] == ESCAPE) {
                return 0;
            } else if (buf[0] == QUIT) {
                return -1;
            }
        }
        if (bytes==3) {
            if (memcmp(buf, UP_ARROW, 3)==0 && cursor>0) {
                cursor--;
                write(STDOUT_FILENO, UP_ARROW, sizeof(UP_ARROW));
            } else if (memcmp(buf, DOWN_ARROW, 3)==0 && cursor<scroll_len-1) {
                cursor++;
                write(STDOUT_FILENO, DOWN_ARROW, sizeof(DOWN_ARROW));
            } 
            if (enable_side_move) {
                if (memcmp(buf, LEFT_ARROW, 3)==0 && cursor>0) {
                    cursor--;
                    write(STDOUT_FILENO, LEFT_ARROW, sizeof(LEFT_ARROW));
                } else if (memcmp(buf, RIGHT_ARROW, 3)==0 && cursor<scroll_len-1) {
                    cursor++;
                    write(STDOUT_FILENO, RIGHT_ARROW, sizeof(RIGHT_ARROW));
                }
            }
        }
    }
}