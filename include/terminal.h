#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include<termios.h>

#define CLEAR_SCREEN "\033[2J"
#define CURSOR_HOME "\033[H"
#define ENTER "\012"
#define ESCAPE "\033"
#define UP_ARROW "\033[A"
#define DOWN_ARROW "\033[B"
#define RIGHT_ARROW "\033[C"
#define LEFT_ARROW "\033[D"


void relocate_cursor(int x, int y);
void clear_terminal();
void canonical_mode();
void non_canonical_mode();
void cursor_movement(int c);

#endif /*_TERMINAL_H_*/