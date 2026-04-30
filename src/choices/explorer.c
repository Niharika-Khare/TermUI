#include "common.h"
#include "explorer.h"
#include "terminal.h"



static char buffer[MAX_ENTRY_CNT][MAX_ENTRY_LENGTH];
int b_start_ind = 0, b_end_ind = 0;
int header_offset = 0;

static int dir_list[MAX_ENTRY_CNT];
static char entry_name_list[MAX_ENTRY_CNT][MAX_ENTRY_LENGTH];

static char nav_history[MAX_HISTORY_CNT][PATH_MAX];
static int nav_hist_cur_ind = 0, nav_hist_end = 0;



static inline int print_header() {

    strcpy(buffer[b_end_ind++], "\n");
    snprintf(buffer[b_end_ind++], MAX_ENTRY_LENGTH, LINE_FORMAT
                                , SNO_HEADER, NAME_HEADER, PERMS_HEADER);

    strcpy(buffer[b_end_ind++], "\n");
    
    return b_end_ind;
}

static inline void print_footer() {

}

static inline void initialize() {

    memset(buffer, 0, sizeof(buffer));
    memset(dir_list, 0, sizeof(dir_list));
    memset(entry_name_list, 0, sizeof(entry_name_list));
    memset(nav_history, 0, sizeof(nav_history));
    b_start_ind = b_end_ind = 0;
    nav_hist_cur_ind = nav_hist_end = 0;

    
    Winsize w;
    get_window_size(&w);

    relocate_cursor(0,0);
    clear_terminal();
}

static int get_entry_info(Dirent* d, char * path) {

    if (b_end_ind >= MAX_ENTRY_CNT) {
        return 0;
    }

    char *entry_name = d->d_name;
    dir_list[b_end_ind] = d->d_type & DT_DIR;
    strcpy(entry_name_list[b_end_ind], entry_name);

    Stat entry_stat;
    char current_path[PATH_MAX];
    snprintf(current_path, PATH_MAX, "%s/%s", path, entry_name);
    stat(current_path, &entry_stat);

    int entry_mode = entry_stat.st_mode;
    int entry_size = entry_stat.st_size;
    int entry_hard_links = entry_stat.st_nlink;

    snprintf(buffer[b_end_ind], MAX_ENTRY_LENGTH, LINE_FORMAT, itoa(b_end_ind - header_offset + 1), entry_name, itoa(entry_mode));
        
    b_end_ind++;
    return 0;
}

static int directory_display (char *path) {
    initialize();
    DIR * d;
    if ((d = opendir(path))) {

        header_offset = print_header();
        Dirent * entry;
        while ((entry = readdir(d))) {
            get_entry_info(entry, path);
        }
        write(STDOUT_FILENO, buffer, sizeof(buffer));
        relocate_cursor(header_offset + 1, 0);
        
        POS cursor_pos = cursor_scroll(b_end_ind - header_offset, ENTER_ENABLED | DIRECTORY_TRAVERSAL);
        if (cursor_pos.c_vert == ESCAPE_CODE || cursor_pos.c_vert == QUIT_CODE) {
            return cursor_pos.c_vert;
        }
        if (dir_list[header_offset + cursor_pos.c_vert]) {
            char current_path[PATH_MAX];
            snprintf(current_path, PATH_MAX, "%s/%s", path, entry_name_list[header_offset + cursor_pos.c_vert]);
            return directory_display(current_path);
        } 
        else {
            // open the file
        }
        return 0;
    }
    return 0;
}

/**
 * 
 * 1. get the list of files and directory at the root and store in a buffer
 *      - this buffer would be displayed at the viewport and hence would 
 *        contain details of the directory content in long format (ls -l)
 * 2. enable cursor scroll with the scroll_len = list size
 *      - if user presses <- or -> then traverse nav_history via the nav_history_index, 
 *        provided it doesn't exceed the nav_histry_index_end
 * 3. the output of cursor scroll will be used to check the buffer entry for file/dir
 *      - if file, open file (fork() + open() + read())
 *            i. clear termianl and display file contents
 *           ii. relocate cursor to 0,0 and enable cursor scroll with horizontal_nav
 *          iii. if user presses esc, clear terminal, display directorys at path
 *      - if dir:
 *            i. check nav_history_index, if it going out of bounds, clear in fifo order
 *           ii. if the nav_history_index in in btw the bounds, reset the nav_histry_index_end 
 *             to current nav_history_index
 *          iii. add current path to the nav_history and the nav_history_index
 *           iv. chdir to current_path + dir and goto 1 (i.e. loop)
 * 
 */
int explorer() {
    int escape_exp = 0;

    char path[PATH_MAX];
    if (getcwd(path, PATH_MAX) == NULL) {
        log_err("Unable to get current working directory\n");
        getc(stdin);
        return ESCAPE_CODE;
    }
    
    do {
        escape_exp = directory_display(path);
    }
    while (escape_exp != ESCAPE_CODE && escape_exp != QUIT_CODE);
    return escape_exp;
}