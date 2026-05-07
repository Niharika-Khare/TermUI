#include "common.h"
#include "history.h"
#include "terminal.h"

static char history_buffer [MAX_APP_HISTORY_CNT] [MAX_ENTRY_LEN];
static int head = 0, hist_cnt = 0 ;

static inline void format_entry (char * entry) {

    int len = strlen (entry) ;
    for ( int i = len-1; i >= 0; i-- ) {

        if ( entry[i] == '\n' ) {
            entry[i] = '\0';
        } 
        else {
            break;
        }
    }
}

void init_history () {
    memset (history_buffer, '\0', sizeof (history_buffer)) ;
}


void save_history (char * entry) {
     
    format_entry (entry) ;
    strcpy (history_buffer[head], entry) ;
    head = (head + 1) % MAX_APP_HISTORY_CNT ;
    hist_cnt++ ;
}

int view_history () {

    Winsize w;
    get_window_size (&w);
    
    POS st = { 1, min (hist_cnt, w.ws_row) } ;
    int offset = 0 ;

    do {
        int ind = hist_cnt > MAX_APP_HISTORY_CNT ? hist_cnt - MAX_APP_HISTORY_CNT + 1: 1;
        int tail = head;
        int count = 0;
        
        clear_terminal ();
        do {
            if (history_buffer[tail][0] != '\0' ) {

                const char * fmt = (count == 0) ? DISABLE_LINE_WRAP "%3d. %s" ENABLE_LINE_WRAP
                                               : DISABLE_LINE_WRAP "\n%3d. %s" ENABLE_LINE_WRAP ;
                dprintf (STDOUT_FILENO, fmt, ind + offset, history_buffer [tail + offset] );
                
                ind++ ;
                count++ ;
            }

            tail = (tail + 1) % MAX_APP_HISTORY_CNT;
        }
        while ( tail != head ) ;
    
        relocate_cursor (st.c_vert, 1) ;
        st = cursor_scroll (st, count, 0) ;

        if (st.c_vert == SCROLL_ONE_DOWN_CODE) {

            if (count > w.ws_row) {
                st.c_vert = w.ws_row ;
                offset = (offset < 0) ? offset + 1 : offset;
            } 
            else {
                st.c_vert = count;
            } 
        }
        else if (st.c_vert == SCROLL_ONE_UP_CODE) {
            st.c_vert = 1;
            offset = (offset > w.ws_row - count) ? offset - 1: offset;
        }
        relocate_cursor (1, 1) ;
    }
    while (st.c_vert != ESCAPE_CODE && st.c_vert != QUIT_CODE) ;

    return st.c_vert;
}