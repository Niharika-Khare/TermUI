#include"terminal.h"

typedef struct termios Terminal;

static Terminal original_term;

static int get_terminal(Terminal *t, int file_no) {
    int tc;
    if ((tc = tcgetattr(file_no, t)) == -1) {
        char err_msg[] = "Failed to get terminal attributes!";
        write(STDERR_FILENO, err_msg, sizeof(err_msg)-1);
    } 
    return tc;
}

static int set_terminal(Terminal *t, int file_no) {
    int tc;
    if ((tc = tcsetattr(file_no, TCSANOW, t)) == -1) {
        char err_msg[] = "Failed to set terminal attributes!";
        write(STDERR_FILENO, err_msg, sizeof(err_msg)-1);
    }
    return tc;
}


/**
 * Get size of current window
 */
void get_window_size(Winsize * w) {
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, w) == -1) {
        char err_msg[] = "Unable to get size of current terminal window\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg)-1);
    }
}

/**
 * Move cursor to the specified coordinates
 * \033[0;0H and \033[1;1H result in same cursor positions i.e start of the screen
 */
void relocate_cursor(int x, int y) {
    char cursor_loc[15];
    int bytes = snprintf(cursor_loc, sizeof(cursor_loc), "\033[%d;%dH", x, y);
    write(STDOUT_FILENO, cursor_loc, bytes);
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
    write(STDOUT_FILENO, clear_terminal_cmd, sizeof(clear_terminal_cmd)-1);
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
    } 
    else {
        Terminal t;
        if (get_terminal(&t, STDIN_FILENO) == -1) {
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
    Terminal t;
    if (get_terminal(&t, STDIN_FILENO) == -1) {
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
 * TODO: add a good explanation of what is happening here
 */
POS cursor_scroll(POS start_pos, int scroll_len, int control_flags) {
    Winsize w;
    get_window_size(&w);
    int vert_lim = scroll_len < w.ws_row ? scroll_len: w.ws_row;
    int horzt_lim = w.ws_col;

    int cursor_ver = start_pos.c_vert, cursor_horz = start_pos.c_horz;
    char buf[10];

    POS pos;
    while (1) {
        int bytes = read(STDIN_FILENO, buf, 10);
        if (bytes == 1) {

            pos.c_horz = cursor_horz;
            pos.c_vert = cursor_ver;

            if (buf[0] == NEW_LINE && control_flags & ENTER_ENABLED) {
                return pos;
            } 
            else if (buf[0] == ESCAPE) {
                pos.c_vert = ESCAPE_CODE;
                return pos;
            } 
            else if (buf[0] == QUIT || buf[0] == _QUIT) {
                pos.c_vert = QUIT_CODE;
                return pos;
            }
        }
        if (bytes == 3) {
            if (memcmp(buf, UP_ARROW, 3) == 0) {

                if (cursor_ver > 1) {
                    cursor_ver--;
                    write(STDOUT_FILENO, UP_ARROW, sizeof(UP_ARROW));
                } 
                else if (cursor_ver == 1) {
                    pos.c_horz = cursor_horz;
                    pos.c_vert = SCROLL_ONE_UP_CODE;
                    return pos;
                }
                
            } 
            if (memcmp(buf, DOWN_ARROW, 3) == 0) {

                if (cursor_ver < vert_lim) {
                    cursor_ver++;
                    write(STDOUT_FILENO, DOWN_ARROW, sizeof(DOWN_ARROW));
                }
                else if (cursor_ver == vert_lim) {
                    pos.c_horz = cursor_horz;
                    pos.c_vert = SCROLL_ONE_DOWN_CODE;
                    return pos;
                }
            } 

            if (control_flags & HORIZONTAL_NAV) {
                if (memcmp(buf, LEFT_ARROW, 3) == 0 && cursor_horz > 1) {
                    cursor_horz--;
                    write(STDOUT_FILENO, LEFT_ARROW, sizeof(LEFT_ARROW));
                } 
                else if (memcmp(buf, RIGHT_ARROW, 3) == 0 && cursor_horz < horzt_lim) {
                    cursor_horz++;
                    write(STDOUT_FILENO, RIGHT_ARROW, sizeof(RIGHT_ARROW));
                }
            } 
            else if (control_flags & DIRECTORY_TRAVERSAL) {

                pos.c_horz = cursor_horz;
                pos.c_vert = cursor_ver;

                if (memcmp(buf, LEFT_ARROW, 3) == 0) {
                    pos.c_horz = LEFT_TRAV_CODE;
                    return pos;
                } 
                else if (memcmp(buf, RIGHT_ARROW, 3) == 0 && cursor_horz < horzt_lim) {
                    pos.c_horz = RIGHT_TRAV_CODE;
                    return pos;
                }
            }
        }
    }
}