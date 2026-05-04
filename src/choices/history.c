#include "common.h"
#include "history.h"
#include "terminal.h"

static char history_buffer[MAX_APP_HISTORY_CNT][MAX_ENTRY_LEN];
static int head = 0, maxed = 1;


static void format_entry(char *entry) {
    int len = strlen(entry);
    for (int i=len-1; i>=0; i--) {
        if (entry[i] == '\n') {
            entry[i] = '\0';
        } else {
            break;
        }
    }
}

void init_history() {
    memset(history_buffer, 0, sizeof(history_buffer));
}

void save_history (char * entry) {
    format_entry(entry);
    snprintf(history_buffer[head], MAX_ENTRY_LEN, DISABLE_LINE_WRAP "%3d. %s\n" ENABLE_LINE_WRAP, head + 1, entry);
    if (++head == MAX_APP_HISTORY_CNT) {
        head = 0;
        maxed = 1;
    }
}

int view_history() {
    clear_terminal();

    Winsize w;
    get_window_size(&w);
    int len = min(MAX_ENTRY_LEN, w.ws_row);
    int header_offset = 0;

    POS st = { maxed == 1? len : head , 1};
    int scr_ind = 0;
    
    do {
        relocate_cursor(1, 1);
        if (maxed == 1) {
            st.c_horz = 1;
            write(STDOUT_FILENO, history_buffer + head * MAX_ENTRY_LEN, MAX_ENTRY_LEN * (MAX_APP_HISTORY_CNT - head));
        }
        write(STDOUT_FILENO, history_buffer, MAX_ENTRY_LEN * head);
        st = cursor_scroll(st, maxed == 1? len : head, 0);
        if (st.c_vert == SCROLL_ONE_DOWN_CODE) {
            st.c_vert = w.ws_row;
            if (scr_ind < sizeof(history_buffer)-len) {
                scr_ind++;
            }
        }
        else if (st.c_vert == SCROLL_ONE_UP_CODE) {
            st.c_vert = 1;
            if (scr_ind > 1) {
                scr_ind--;
            }
        }
    }
    while (st.c_vert != ESCAPE_CODE && st.c_vert != QUIT_CODE);

    return st.c_vert;
}