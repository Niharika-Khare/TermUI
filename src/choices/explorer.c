#include "common.h"
#include "explorer.h"
#include "terminal.h"



Winsize w;
static char buffer[MAX_ENTRY_CNT][MAX_ENTRY_LENGTH];
int b_start_ind = 1, b_end_ind = 1;
int header_offset = 0;

static int dir_list[MAX_ENTRY_CNT];
static char entry_name_list[MAX_ENTRY_CNT][MAX_ENTRY_LENGTH];

static char nav_history[MAX_HISTORY_CNT][PATH_MAX];
static int nav_hist_cur = 0, nav_hist_end = 0;


static inline int print_header(char * path) {

    char h_buffer[MAX_HEADER_CNT][MAX_ENTRY_LENGTH];
    memset(h_buffer, 0, sizeof(h_buffer));
    int h_offset = 0;

    snprintf(h_buffer[h_offset++], MAX_ENTRY_LENGTH, "Directory Path: %s", path);
    snprintf(h_buffer[h_offset++], MAX_ENTRY_LENGTH, DISABLE_LINE_WRAP LINE_FORMAT ENABLE_LINE_WRAP
                                 , SNO_HEADER, NAME_HEADER, SIZE_HEADER, PERMS_HEADER
                                 , USER_NAME_HEADER, GROUP_NAME_HEADER, MTIME_HEADER, HARD_LINK_HEADER);
    write(STDOUT_FILENO, h_buffer, sizeof(h_buffer));
    return h_offset;
}

static inline void initialize() {

    memset(buffer, 0, sizeof(buffer));
    memset(dir_list, 0, sizeof(dir_list));
    memset(entry_name_list, 0, sizeof(entry_name_list));
    b_start_ind = b_end_ind = 1;
    get_window_size(&w);

    relocate_cursor(0,0);
    clear_terminal();
}

static void h_perm(mode_t mode, char *h_perm) {
    memcpy(h_perm, "---------", 11);

    if (S_ISDIR(mode))  h_perm[0] = 'd';       // Directory
    else if (S_ISLNK(mode))  h_perm[0] = 'l';  // Symlink
    else if (S_ISCHR(mode))  h_perm[0] = 'c';  // Character device
    else if (S_ISBLK(mode))  h_perm[0] = 'b';  // Block device
    else if (S_ISFIFO(mode)) h_perm[0] = 'p';  // FIFO / Pipe
    else if (S_ISSOCK(mode)) h_perm[0] = 's';  // Socket

    if (mode & S_IRUSR) h_perm[1] = 'r';
    if (mode & S_IWUSR) h_perm[2] = 'w';
    if (mode & S_IXUSR) h_perm[3] = 'x';

    if (mode & S_IRGRP) h_perm[4] = 'r';
    if (mode & S_IWGRP) h_perm[5] = 'w';
    if (mode & S_IXGRP) h_perm[6] = 'x';

    if (mode & S_IROTH) h_perm[7] = 'r';
    if (mode & S_IWOTH) h_perm[8] = 'w';
    if (mode & S_IXOTH) h_perm[9] = 'x';

    h_perm[10] = '\0';
}

static void h_size(size_t size, char * h_size) {

    const char *units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    int i = 0;
    memset(h_size, 0, sizeof(size_t));
    double display_size = (double) size;

    while (display_size >= 1024.0 && i < 5) {
        display_size /= 1024.0;
        i++;
    }
    if (i == 0) {
        snprintf(h_size, sizeof(size_t), "%zu%s", size, units[i]);
    } else {
        snprintf(h_size, sizeof(size_t), "%.2f%s", display_size, units[i]);
    }

}

static int get_entry_info(Dirent* d, char * path) {

    if (b_end_ind >= MAX_ENTRY_CNT) {
        return 0;
    }

    char *entry_name = d->d_name;
    dir_list[b_end_ind] = d->d_type & DT_DIR;
    strcpy(entry_name_list[b_end_ind], entry_name);

    Stat entry_stat;
    char full_path[PATH_MAX];
    snprintf(full_path, PATH_MAX, "%s/%s", path, entry_name);
    stat(full_path, &entry_stat);
    Pwd * p = getpwuid(entry_stat.st_uid);
    Grp * g = getgrgid(entry_stat.st_gid);

    int entry_size = entry_stat.st_size;

    char entry_h_perm[11];
    h_perm(entry_stat.st_mode, entry_h_perm);

    char entry_h_size[sizeof(size_t)];
    h_size(entry_stat.st_size, entry_h_size);

    char *user_name = p ? p->pw_name : "-" ;
    char *grp_name = g ? g->gr_name : "-" ;
    char *mod_time = ctime(&entry_stat.st_mtime);

    snprintf(buffer[b_end_ind], MAX_ENTRY_LENGTH, DISABLE_LINE_WRAP LINE_FORMAT ENABLE_LINE_WRAP, 
                                    itoa(b_end_ind), entry_name, entry_h_size, entry_h_perm,
                                    user_name, grp_name, mod_time, itoa(entry_stat.st_nlink));
        
    b_end_ind++;
    return 0;
}

static inline char * get_current_path(char * new_path, char * old_path, char * dir_name) {

    int p_len = strlen(old_path);
    int d_len = strlen(dir_name);

    if (memcmp(dir_name, "..", sizeof ("..")) == 0) {
        int i;
        for (i = p_len-1; i >= 0 && old_path[i] != '/'; i--);
        if (i) {
            memcpy(new_path, old_path, i);
            new_path[i] = '\0';
        }
        else {
            new_path[0] = '/';
            new_path[1] = '\0';
        }
    } 
    else if (memcmp(dir_name, ".", sizeof (".")) == 0) {       
        memcpy(new_path, old_path, p_len);
        new_path[p_len] = '\0';
    } 
    else if (p_len == 1) {   
        new_path[0] = '/';                                  
        memcpy(new_path + 1, dir_name, d_len);
        new_path[1 + d_len] = '\0';
    }
    else {
        memcpy(new_path, old_path, p_len);
        new_path[p_len] = '/';
        memcpy(new_path + p_len + 1, dir_name, d_len);
        new_path[p_len + 1 + d_len] = '\0';
    }
    return new_path;
}

static void store_nav_history(char * path) {
    nav_hist_end = nav_hist_cur + 1;
    strcpy(nav_history[nav_hist_end++], path);
    nav_hist_cur = nav_hist_end - 1;
}

// TODO: add a good explanation of what is happening here
static int directory_display (char *path) {
    initialize();
    DIR * d;
    if ((d = opendir(path))) {

        header_offset = print_header(path);
        Dirent * entry;
        while ((entry = readdir(d))) {
            get_entry_info(entry, path);
        }
        POS cursor_start = {1, 1};
        int cursor_screen_pos = header_offset + 1;
        do {
            int scroll_len = min(w.ws_row - header_offset, b_end_ind - b_start_ind);
            write(STDOUT_FILENO, &buffer[b_start_ind], sizeof(buffer[0]) * scroll_len);
            relocate_cursor(cursor_screen_pos, 1);

            POS cursor_pos = cursor_scroll(cursor_start, scroll_len, ENTER_ENABLED | DIRECTORY_TRAVERSAL);

            if (cursor_pos.c_vert == ESCAPE_CODE || cursor_pos.c_vert == QUIT_CODE) {
                return cursor_pos.c_vert;
            }
            else if (cursor_pos.c_horz == LEFT_TRAV_CODE) {
                if (nav_hist_cur > 1) {
                    nav_hist_cur--;
                }
                return directory_display(nav_history[nav_hist_cur]);
            }
            else if (cursor_pos.c_horz == RIGHT_TRAV_CODE) {

                if (nav_hist_cur < nav_hist_end - 1) {
                    nav_hist_cur++;
                }
                return directory_display(nav_history[nav_hist_cur]);
            }
            else if (cursor_pos.c_vert == SCROLL_ONE_UP_CODE) {
                relocate_cursor(header_offset, 0);
                if (b_start_ind > 0) {
                    b_start_ind--;
                    cursor_screen_pos = header_offset + 1;;
                    cursor_start.c_vert = 1;
                    cursor_start.c_horz = 1;
                }
            }
            else if (cursor_pos.c_vert == SCROLL_ONE_DOWN_CODE) {
                relocate_cursor(header_offset, 0);
                if (b_end_ind - b_start_ind > w.ws_row - header_offset) {
                    b_start_ind++;
                    cursor_screen_pos = w.ws_row;
                    cursor_start.c_vert = cursor_screen_pos - header_offset;
                    cursor_start.c_horz = 1;
                }
            }
            else if (cursor_pos.c_vert >= 0 && cursor_pos.c_horz >= 0) {
                int entry_ind = cursor_pos.c_vert + b_start_ind - 1;
                if (dir_list[entry_ind]) {
                    char current_path[PATH_MAX];
                    get_current_path(current_path, path, entry_name_list[entry_ind]);
                    store_nav_history(current_path);
                    return directory_display(current_path);
                }
                else {
                    clear_terminal();
                    char *f_name = entry_name_list[entry_ind];
                    pid_t pid = fork();
                    if (pid < 0) {
                        log_err("err: fork: unable to open file %s", f_name);
                    }
                    else if (pid == 0) {
                        char file_buffer[FILE_BLOCK_SIZE];
                        
                        int bytes = 0, fd;
                        if ((fd = open(f_name, O_RDONLY)) != -1) {
                            while ((bytes = read(fd, file_buffer, FILE_BLOCK_SIZE))) {
                                write(STDOUT_FILENO, file_buffer, bytes);
                            }
                            close(fd);
                        } else {
                            log_err("err: unable to open file: %s\n", f_name);
                        }
                        relocate_cursor(1, 1);
                        POS st = {1, 1};
                        do {
                            st = cursor_scroll(st, w.ws_row, HORIZONTAL_NAV);
                            if (st.c_vert == SCROLL_ONE_UP_CODE) {
                                st.c_vert = 1;
                            }
                            else if (st.c_vert == SCROLL_ONE_DOWN_CODE) {
                                st.c_vert = w.ws_row;
                            }
                        }
                        while (st.c_vert != ESCAPE_CODE && st.c_vert != QUIT_CODE);
                        exit(0);
                    } 
                    else {
                        wait(NULL);
                        return directory_display(path);
                    } 
                }
            }
        } 
        while (1);
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
    memset(nav_history, 0, sizeof(nav_history));

    char path[PATH_MAX];
    if (getcwd(path, PATH_MAX) == NULL) {
        log_err("Unable to get current working directory\n");
        getc(stdin);
        return ESCAPE_CODE;
    }

    do {
        store_nav_history(path);
        escape_exp = directory_display(path);
    }
    while (escape_exp != ESCAPE_CODE && escape_exp != QUIT_CODE);
    return escape_exp;
}