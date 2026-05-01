#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * ANSI escape sequences
 */
#define DISABLE_LINE_WRAP       "\033[?7l"
#define ENABLE_LINE_WRAP        "\033[?7h"
#define CURSOR_TO_END_CLEAR     "\033[J"    
#define CLEAR_SCREEN            "\033[2J"
#define CLEAR_SCROLLBACK        "\033[3J"
#define CURSOR_HOME             "\033[H"
#define NEW_LINE                '\012'
#define ESCAPE                  '\033'
#define UP_ARROW                "\033[A"
#define DOWN_ARROW              "\033[B"
#define RIGHT_ARROW             "\033[C"
#define LEFT_ARROW              "\033[D"
#define QUIT                    'q'
#define _QUIT                   'q'

/**
 * Application specific codes
 */
#define LEFT_TRAV_CODE          -1
#define RIGHT_TRAV_CODE         -2
#define SCROLL_ONE_UP_CODE      -3
#define SCROLL_ONE_DOWN_CODE    -4
#define ESCAPE_CODE             -5
#define QUIT_CODE               -6

/**
 * Control flags for keyboard actions
 */
#define HORIZONTAL_NAV          1   
#define DIRECTORY_TRAVERSAL     2 
#define ENTER_ENABLED           4 


typedef struct winsize Winsize;

typedef struct pos {
    int c_horz;
    int c_vert;
} POS;


void relocate_cursor(int x, int y);
void clear_terminal();
void get_window_size(Winsize * w);
int canonical_mode();
int non_canonical_mode();
POS cursor_scroll(POS start, int scroll_len, int control_flags);

#endif /*_TERMINAL_H_*/