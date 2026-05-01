#ifndef _EXPLORER_
#define _EXPLORER_

#include <dirent.h>
#include <limits.h>
#include <sys/wait.h>
#include <fcntl.h>


#define MAX_ENTRY_CNT               100
#define MAX_HEADER_CNT              5
#define MAX_ENTRY_LENGTH            512
#define MAX_HISTORY_CNT             30


#define FILE_BLOCK_SIZE             2048
typedef struct dirent Dirent;
typedef struct stat Stat;


#define LINE_FORMAT             "\n%4s.\t%20s\t%15s"
#define SNO_HEADER              "S.no"
#define NAME_HEADER             "Name"
#define PERMS_HEADER            "Permissions"


int explorer();


#endif /* _EXPLORER_ */