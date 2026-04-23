#ifndef _HOME_H_
#define _HOME_H_

/* Choice menu */
#define SYS_INFO "System Information"
#define EXPLR    "File Explorer"
#define N_SHELL  "nShell"
#define HISTORY  "Session history"
#define HELP     "Help"

#define MAX_HOME_SCREEN_BUFFER 1000

typedef struct {
    int scroll_start;
    int choice_cnt;
} MessageProp;

int render_home_screen(MessageProp* prop);
int process_choice(int choice, int choice_cnt);

#endif /*_HOME_H_ */