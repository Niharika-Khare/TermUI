#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include<sys/ioctl.h>
#include<termios.h>

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

/**
 * Control flags for keyboard actions
 */
#define HORIZONTAL_NAV          1   
#define DIRECTORY_TRAVERSAL     2 
#define NEW_LINE_ENABLED        4 

typedef struct winsize Winsize;

void relocate_cursor(int x, int y);
void clear_terminal();
void get_window_size(Winsize * w);
int canonical_mode();
int non_canonical_mode();
int cursor_scroll(int scroll_len, int control_flags);

#endif /*_TERMINAL_H_*/