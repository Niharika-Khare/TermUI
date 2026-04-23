#ifndef _SYS_INFO_
#define _SYS_INFO_

#include<sys/sysctl.h>
#include<sys/utsname.h>

typedef struct utsname OsInfo;

int system_info();


#endif /* _SYS_INFO_ */