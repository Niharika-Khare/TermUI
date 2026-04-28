#ifndef _MK_DIR_
#define _MK_DIR_

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>


/* Valid flags*/
#define M_                  1
#define V_                  2


/* Limits and defaults*/
#define MAX_DIR_COUNT       100
#define DEFAULT_MODE        0644    /* rw-r--r-- */



#endif /* _MK_DIR_ */