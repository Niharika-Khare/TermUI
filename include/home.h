#ifndef _HOME_H_
#define _HOME_H_

#define SYS_INFO "System Information"
#define EXPLR    "File Explorer"
#define N_SHELL  "nShell"
#define HISTORY  "Session history"
#define HELP     "Help"

typedef struct {
    int scroll_start;
    int choice_cnt;
} MessageProp;

void render_home_screen(MessageProp* prop);
int process_choice(int choice, int choice_cnt);

#endif /*_HOME_H_ */