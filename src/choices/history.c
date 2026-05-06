#include "common.h"
#include "history.h"
#include "terminal.h"

static char history_buffer [MAX_APP_HISTORY_CNT] [MAX_ENTRY_LEN];
static int head = 0, hist_cnt = 0;

static inline void format_entry(char * entry) {

    int len = strlen(entry);
    for (int i=len-1; i>=0; i--) {

        if (entry[i] == '\n') {
            entry[i] = '\0';
        } 
        else {
            break;
        }
    }
}

void init_history() {
    memset ( history_buffer, '\0', sizeof(history_buffer) ) ;
}


void save_history (char * entry) {
     
    format_entry(entry) ;
    strcpy ( history_buffer[head], entry ) ;
    head = (head + 1) % MAX_APP_HISTORY_CNT;
    hist_cnt++;
}

int view_history() {

    Winsize w;
    get_window_size(&w);
    
    char * print_format = DISABLE_LINE_WRAP "%3d. %s\n" ENABLE_LINE_WRAP;
    char print_buffer [MAX_APP_HISTORY_CNT] [MAX_ENTRY_LEN + sizeof (print_format) + 1];

    POS st;
    st.c_vert =  min(hist_cnt, w.ws_row);
    st.c_horz =  1;
    do {
        int ind = hist_cnt > w.ws_row ? hist_cnt - w.ws_row : 0;
        memset ( print_buffer, '\0', sizeof(print_buffer) ) ;

        int tail = head;
        int  p_i = 0, p_st = 0;

        do {
            if (history_buffer[tail][0] != '\0' ) {
                
                snprintf ( print_buffer [p_i], 
                       MAX_ENTRY_LEN + sizeof (print_format) + 1, 
                       print_format,
                       ind + 1, 
                       history_buffer[tail]) ;

                if (p_i > w.ws_row) {
                    p_st++ ;
                } 
                ind++ ;
                p_i++ ;
            }

            tail = (tail + 1) % MAX_APP_HISTORY_CNT;
        }
        while ( tail != head );

    
        clear_terminal();
        write ( STDOUT_FILENO , 
                print_buffer + p_st , 
                (p_i - p_st + 1) * (MAX_ENTRY_LEN + sizeof (print_format) + 1) ) ;

    
        relocate_cursor (st.c_vert, 1) ;
        st = cursor_scroll (st, p_i, 0) ;

        if (st.c_vert == SCROLL_ONE_DOWN_CODE) {
            st.c_vert = min (p_i, w.ws_row);
            // p_st = p_st < p_i - st.c_vert ? p_st + 1 : p_st ;
        }
        else if (st.c_vert == SCROLL_ONE_UP_CODE) {
            st.c_vert = 1;
            // p_st = p_st > 0 ? p_st - 1 : p_st ; 
        }
        relocate_cursor (1, 1) ;
    }
    while (st.c_vert != ESCAPE_CODE && st.c_vert != QUIT_CODE);

    return st.c_vert;
}