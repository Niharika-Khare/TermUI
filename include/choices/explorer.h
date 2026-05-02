#ifndef _EXPLORER_
#define _EXPLORER_

#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <sys/wait.h>


#define MAX_ENTRY_CNT               100
#define MAX_HEADER_CNT              5
#define MAX_ENTRY_LENGTH            512
#define MAX_HISTORY_CNT             30
#define FILE_BLOCK_SIZE             2048



typedef struct dirent Dirent;
typedef struct stat Stat;
typedef struct passwd Pwd;
typedef struct group Grp;


/* S.No    Name    Size    Permissions    User Name    Group Name    Modification Time    H-Link*/
#define LINE_FORMAT             "\n%4s.\t%-20.20s\t%-10.10s\t%-15.15s\t%-20.20s\t%-20.20s\t%-24.24s\t%-5.5s"

#define SNO_HEADER              "S.no"
#define NAME_HEADER             "Name"
#define SIZE_HEADER             "Size"
#define PERMS_HEADER            "Permissions"
#define USER_NAME_HEADER        "User Name"
#define GROUP_NAME_HEADER       "Group Name"
#define MTIME_HEADER            "Modification Time"
#define HARD_LINK_HEADER        "HLink"



int explorer();


#endif /* _EXPLORER_ */