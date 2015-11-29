/* This file contains the main program of the SCHED scheduler. It will sit idle
 * until asked, by PM, to take over scheduling a particular process.
 */

/* The _MAIN def indicates that we want the schedproc structs to be created
 * here. Used from within schedproc.h */
#define _MAIN

#include "sched.h"
#include "schedproc.h"

/* Declare some local functions. */
static void reply(endpoint_t whom, message *m_ptr);
static void sef_local_startup(void);
static int sef_cb_init_fresh(int type, sef_init_info_t *info);
static void handle_schedswitch(message *m_in);

struct machine machine;         /* machine info */

int verbose = 0;                /* verbose = 1 prints debug info to console  */
int start_quantum = 200;        /* the initial quantum for new processes     */
int sched_algorithm = 0;        /* 3 = lottery, all others is stock priority */
int lottery_initialized = 0;    /* 0 = not initialized                       */

/*===========================================================================*
 *                              main                                         *
 *===========================================================================*/
int main(void)
{
	/* Main routine of the scheduler. */
       message m_in;   /* the incoming message itself is kept here. */
       int call_nr;    /* system call number */
       int who_e;      /* caller's endpoint */
       int result;     /* result to system call */
	int rv;

	/* SEF local startup. */
	sef_local_startup();

	/* This is SCHED's main loop - get work and do it, forever and forever. */
	while (TRUE) {
		int ipc_status;

		/* Wait for the next message and extract useful information from it. */
		if (sef_receive_status(ANY, &m_in, &ipc_status) != OK)
			panic("SCHED sef_receive error");
               who_e = m_in.m_source;  /* who sent the message */
               call_nr = m_in.m_type;  /* system call number */

		/* Check for system notifications first. Special cases. */
		if (is_ipc_notify(ipc_status)) {
			switch(who_e) {
                               case CLOCK:
                                       balance_queues();
                                       break;
                               default :
                                       break;
			}

			continue; /* Don't reply. */
		}

		switch(call_nr) {
                       case SCHEDULING_INHERIT:
                       case SCHEDULING_START:
                               result = do_start_scheduling(&m_in);
                               break;
                       case SCHEDULING_STOP:
                               result = do_stop_scheduling(&m_in);
                               break;
                       case SCHEDULING_SET_NICE:
                               result = do_nice(&m_in);
                               break;
                       case SCHEDULING_NO_QUANTUM:
                               /* This message was sent from the kernel, don't reply */
                               if (IPC_STATUS_FLAGS_TEST(ipc_status,
                                                       IPC_FLG_MSG_FROM_KERNEL)) {
                                       if ((rv = do_noquantum(&m_in)) != (OK)) {
                                               printf("SCHED: Warning, do_noquantum "
                                                               "failed with %d\n", rv);
                                       }
                                       continue; /* Don't reply */
				}
                               else {
                                       printf("SCHED: process %d faked "
                                                       "SCHEDULING_NO_QUANTUM message!\n",
                                                       who_e);
                                       result = EPERM;
                               }
                               break;
                       case SCHEDULING_SWITCH:
                               handle_schedswitch(&m_in);
                               break;
                       default:
                               result = no_sys(who_e, call_nr);
		}

		/* Send reply. */
		if (result != SUSPEND) {
                       m_in.m_type = result;           /* build reply message */
                       reply(who_e, &m_in);            /* send it away */
		}
       }
	return(OK);
}


/*===========================================================================*
 *                              handle_schedswitch                           *
 *===========================================================================*/
static void handle_schedswitch(message *msg) 
{
       printf("Scheduling switch message received! message=%d, arg=%d\n",
                       msg->m_lc_pm_schedswitch.sched_mesg, 
                       msg->m_lc_pm_schedswitch.sched_mesg_arg);
       switch(msg->m_lc_pm_schedswitch.sched_mesg) {
               case SCTL_QUANTUM_SET:
                       // TODO should really change all the existing procs?
                       start_quantum = msg->m_lc_pm_schedswitch.sched_mesg_arg;
                       break;
               case SCTL_VERBOSE_SET:
                       verbose = msg->m_lc_pm_schedswitch.sched_mesg_arg;
                       break;
               case SCTL_ALGORITHM_SET:
                       sched_algorithm = msg->m_lc_pm_schedswitch.sched_mesg_arg;
                       if ( sched_algorithm == 3 && lottery_initialized == 0) {
                               init_lottery_sched();
                       } else if (sched_algorithm != 3) {
                               clear_lottery();
                       } 
                       break;
               default:
                       printf("Unknown message!\n");
       }
       printf("[v04] Current Settings:\n");
       printf("  verbose: %d\n", verbose);
       printf("  quantum: %d\n", start_quantum);
       printf("  algorithm: %d\n", sched_algorithm);
}

/*===========================================================================*
 *                              reply                                        *
 *===========================================================================*/
static void reply(endpoint_t who_e, message *m_ptr)
{
	int s = ipc_send(who_e, m_ptr);    /* send the message */
	if (OK != s)
		printf("SCHED: unable to send reply to %d: %d\n", who_e, s);
}

/*===========================================================================*
 *                             sef_local_startup                             *
 *===========================================================================*/
static void sef_local_startup(void)
{
	/* Register init callbacks. */
	sef_setcb_init_fresh(sef_cb_init_fresh);
	sef_setcb_init_restart(SEF_CB_INIT_RESTART_STATEFUL);

	/* No signal callbacks for now. */

	/* Let SEF perform startup. */
	sef_startup();
}

/*===========================================================================*
 *                          sef_cb_init_fresh                                *
 *===========================================================================*/
static int sef_cb_init_fresh(int UNUSED(type), sef_init_info_t *UNUSED(info))
{
	int s;

	if (OK != (s=sys_getmachine(&machine)))
		panic("couldn't get machine info: %d", s);
	/* Initialize scheduling timers, used for running balance_queues */
	init_scheduling();

	return(OK);
}


