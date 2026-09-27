#include "stdafx.h"

#ifdef OS_WINDOWS
// Windows has no SIGTERM: a graceful shutdown is requested by setting the per-process
// named event "Local\mt2_shutdown_<pid>", which has the same effect as SIGTERM on POSIX.
static HANDLE s_hShutdownEvent = NULL;

void signal_setup()
{
	char szEventName[64];
	snprintf(szEventName, sizeof(szEventName), "Local\\mt2_shutdown_%lu", GetCurrentProcessId());

	s_hShutdownEvent = CreateEventA(NULL, TRUE, FALSE, szEventName);
	if (!s_hShutdownEvent)
		sys_err("Cannot create shutdown event %s (error %lu)", szEventName, GetLastError());
}

void signal_poll_shutdown()
{
	if (s_hShutdownEvent && !shutdowned.load() && WaitForSingleObject(s_hShutdownEvent, 0) == WAIT_OBJECT_0)
	{
		shutdowned = TRUE;
		sys_err("Shutdown event has been received. shutting down.");
	}
}

void signal_timer_disable() {}
void signal_timer_enable(int timeout_seconds) {}
#else
void signal_poll_shutdown() {}

#define RETSIGTYPE void

RETSIGTYPE reap(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
    signal(SIGCHLD, reap);
}


RETSIGTYPE checkpointing(int sig)
{
    if (!tics.load())
    {
        sys_err("CHECKPOINT shutdown: tics did not updated.");
        abort();
    }
    else
	    tics.store(0);
}


RETSIGTYPE hupsig(int sig)
{
    shutdowned = TRUE;
    sys_err("SIGHUP, SIGINT, SIGTERM signal has been received. shutting down.");
}

RETSIGTYPE usrsig(int sig)
{
    core_dump();
}

void signal_timer_disable(void)
{
    struct itimerval itime;
    struct timeval interval;

    interval.tv_sec	= 0;
    interval.tv_usec	= 0;

    itime.it_interval = interval;
    itime.it_value = interval;

    setitimer(ITIMER_VIRTUAL, &itime, NULL);
}

void signal_timer_enable(int sec)
{
    struct itimerval itime;
    struct timeval interval;

    interval.tv_sec	= sec;
    interval.tv_usec	= 0;

    itime.it_interval = interval;
    itime.it_value = interval;

    setitimer(ITIMER_VIRTUAL, &itime, NULL);
}

void signal_setup(void)
{
    signal_timer_enable(30);

    signal(SIGVTALRM, checkpointing);

    /* just to be on the safe side: */
    signal(SIGHUP, hupsig);
    signal(SIGCHLD, reap);
    signal(SIGINT, hupsig);
    signal(SIGTERM, hupsig);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
    signal(SIGUSR1, usrsig);
}

#endif
