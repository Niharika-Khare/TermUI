#ifndef _HISTORY_
#define _HISTORY_


#define MAX_APP_HISTORY_CNT     1000
#define MAX_ENTRY_LEN           200

void init_history();
void save_history(char * entry);
int view_history();

#endif /* _HISTORY_ */