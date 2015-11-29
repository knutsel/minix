/* This is the master header for the Scheduler.  It includes some other files
 * and defines the principal constants.
 */
#define _SYSTEM		1	/* tell headers that this is the kernel */

/* The following are so basic, all the *.c files get them automatically. */
#include <minix/config.h>	/* MUST be first */
#include <sys/types.h>
#include <minix/const.h>

#include <minix/syslib.h>
#include <minix/sysutil.h>

#include <errno.h>

#include "proto.h"

#define SCTL_QUANTUM_SET 1
#define SCTL_VERBOSE_SET 2
#define SCTL_ALGORITHM_SET 3
#define LOTTERY_ALGORITHM 3

extern struct machine machine;		/* machine info */
