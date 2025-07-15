#include "quip_config.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

#include "rt_sched.h"
#include "quip_prot.h"

#ifdef ALLOW_RT_SCHED
int try_rt_sched=1;
int rt_is_on=0;

static int curr_policy=SCHED_OTHER;
#endif // ALLOW_RT_SCHED

#include "unix_prot.h"

void rt_sched(QSP_ARG_DECL  int flag)
{
#ifdef HAVE_SCHED_SETSCHEDULER
#ifdef ALLOW_RT_SCHED
	pid_t pid;
	struct sched_param p;

	pid = getpid();

	if( flag ){		/* enable real-time scheduling */

		p.sched_priority = 1;
		if( sched_setscheduler(pid,SCHED_FIFO,&p) < 0 ){
			perror("sched_setscheduler");
			warn("Unable to set real-time priority (run as root!)");

			rt_is_on = 0;
		} else {
			rt_is_on = 1;
			curr_policy=SCHED_FIFO;
		}
	} else {		/* disable real-time scheduling */
		if( rt_is_on ){
			p.sched_priority = 0;
			if( sched_setscheduler(pid,SCHED_OTHER,&p) < 0 ){
				perror("sched_setscheduler");
			
				warn("Unable to reset real-time priority???");
			}
			rt_is_on = 0;
			curr_policy=SCHED_OTHER;
		}
	}
#endif /* ALLOW_RT_SCHED */
#else // ! HAVE_SCHED_SETSCHEDULER
	warn("rt_sched:  no scheduler support on this system.");
#endif // ! HAVE_SCHED_SETSCHEDULER
}

static COMMAND_FUNC( do_rt_sched )
{
	int yn;

	yn = ASKIF("enable real-time scheduling");
	rt_sched(QSP_ARG  yn);
}

static COMMAND_FUNC( do_get_pri )
{
#ifdef HAVE_SCHED_GETPARAM
#ifdef ALLOW_RT_SCHED
	int min,max;
	struct sched_param p;

	if( sched_getparam(getpid(),&p) < 0 ){
		perror("sched_getparam");
		warn("unable to get scheduler params");
	}

	min = sched_get_priority_min(curr_policy);
	max = sched_get_priority_max(curr_policy);
	snprintf(msg_str,LLEN,"Priority is %d (range %d-%d)" ,p.sched_priority,min,max);
	prt_msg(msg_str);
#endif /* ALLOW_RT_SCHED */
#else // ! HAVE_SCHED_GETPARAM
	warn("do_get_pri:  no scheduler support on this system.");
#endif // ! HAVE_SCHED_GETPARAM
}

static COMMAND_FUNC( do_set_pri )
{

#ifdef HAVE_SCHED_SETPARAM
	int pri;
	int min,max;
	struct sched_param p;
	pid_t pid;
    
	min = sched_get_priority_min(curr_policy);
	max = sched_get_priority_max(curr_policy);
	snprintf(msg_str,LLEN,"priority (%d-%d)",min,max);
	pri = HOW_MANY(msg_str);
	if( pri < min || pri > max ){
		snprintf(ERROR_STRING,LLEN,
                "do_set_pri:  priority (%d) must be in the range %d-%d!?",
                pri,min,max);
		warn(ERROR_STRING);
		return;
	}

#ifdef ALLOW_RT_SCHED
	pid = getpid();
	if( sched_getparam(pid,&p) < 0 ){
		perror("sched_getparam");
		warn("unable to get scheduler parameters!?");
	}
	p.sched_priority = pri;
	if( sched_setparam(pid, &p) < 0 ){
		perror("sched_setparam");
		warn("unable to set priority!?");
	}

#else /* ! ALLOW_RT_SCHED */
	warn("do_set_pri:  scheduler control not allowed in this build!?");
#endif /* ! ALLOW_RT_SCHED */

#else // ! HAVE_SCHED_SETPARAM
	/*pri = (int)*/ HOW_MANY("priority (ineffective)");
	warn("do_set_pri:  no scheduler support on this system.");
#endif // ! HAVE_SCHED_SETPARAM
}


#define ADD_CMD(s,f,h)	ADD_COMMAND(sched_menu,s,f,h)

MENU_BEGIN(sched)
ADD_CMD( rt_sched,	do_rt_sched,	enable/disable real-time scheduling )
ADD_CMD( get_pri,	do_get_pri,	report current priority )
ADD_CMD( set_pri,	do_set_pri,	set priority )
MENU_END(sched)


COMMAND_FUNC( do_sched_menu )
{
	CHECK_AND_PUSH_MENU(sched);
}

