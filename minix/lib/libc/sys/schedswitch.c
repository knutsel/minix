/*
schedswitch.c
*/

#include <sys/cdefs.h>
#include "namespace.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <lib.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>


int schedswitch(int which, id_t who, int prio)
{
       message m;

       memset(&m, 0, sizeof(m));
       m.m_lc_pm_priority.which = which;
       m.m_lc_pm_priority.who = who;
       m.m_lc_pm_priority.prio = prio;

       return _syscall(PM_PROC_NR, PM_SCHEDSWITCH, &m);
}

