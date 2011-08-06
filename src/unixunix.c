/*	SCCS Id: @(#)unixunix.c	3.4	1994/11/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* This file collects some Unix dependencies */

#include "hack.h"	/* mainly for index() which depends on BSD */

#include <errno.h>
#include <sys/stat.h>
#if defined(NO_FILE_LINKS) || defined(SUNOS4) || defined(POSIX_TYPES)
#include <fcntl.h>
#endif
#include <signal.h>

#ifdef _M_UNIX
extern void NDECL(sco_mapon);
extern void NDECL(sco_mapoff);
#endif
#ifdef __linux__
extern void NDECL(linux_mapon);
extern void NDECL(linux_mapoff);
#endif

void
regularize(s)	/* normalize file name - we don't like .'s, /'s, spaces */
register char *s;
{
	register char *lp;

	while((lp=index(s, '.')) || (lp=index(s, '/')) || (lp=index(s,' ')))
		*lp = '_';
#if defined(SYSV) && !defined(AIX_31) && !defined(SVR4) && !defined(LINUX) && !defined(__APPLE__)
	/* avoid problems with 14 character file name limit */
# ifdef COMPRESS
	/* leave room for .e from error and .Z from compress appended to
	 * save files */
	{
#  ifdef COMPRESS_EXTENSION
	    int i = 12 - strlen(COMPRESS_EXTENSION);
#  else
	    int i = 10;		/* should never happen... */
#  endif
	    if(strlen(s) > i)
		s[i] = '\0';
	}
# else
	if(strlen(s) > 11)
		/* leave room for .nn appended to level files */
		s[11] = '\0';
# endif
#endif
}

#if defined(TIMED_DELAY) && !defined(msleep) && defined(SYSV)
#include <poll.h>

void
msleep(msec)
unsigned msec;				/* milliseconds */
{
	struct pollfd unused;
	int msecs = msec;		/* poll API is signed */

	if (msecs < 0) msecs = 0;	/* avoid infinite sleep */
	(void) poll(&unused, (unsigned long)0, msecs);
}
#endif /* TIMED_DELAY for SYSV */

#ifdef SHELL
int
dosh()
{
	register char *str;
	if(child(0)) {
		if((str = getenv("SHELL")) != (char*)0)
			(void) execl(str, str, (char *)0);
		else
			(void) execl("/bin/sh", "sh", (char *)0);
		raw_print("sh: cannot execute.");
		exit(EXIT_FAILURE);
	}
	return 0;
}
#endif /* SHELL */

#if defined(SHELL) || defined(DEF_PAGER) || defined(DEF_MAILREADER)
int
child(wt)
int wt;
{
	register int f;
	suspend_nhwindows((char *)0);	/* also calls end_screen() */
#ifdef _M_UNIX
	sco_mapon();
#endif
#ifdef __linux__
	linux_mapon();
#endif
	if((f = fork()) == 0){		/* child */
		(void) setgid(getgid());
		(void) setuid(getuid());
#ifdef CHDIR
		(void) chdir(getenv("HOME"));
#endif
		return(1);
	}
	if(f == -1) {	/* cannot fork */
		pline("Fork failed.  Try again.");
		return(0);
	}
	/* fork succeeded; wait for child to exit */
	(void) signal(SIGINT,SIG_IGN);
	(void) signal(SIGQUIT,SIG_IGN);
	(void) wait( (int *) 0);
#ifdef _M_UNIX
	sco_mapoff();
#endif
#ifdef __linux__
	linux_mapoff();
#endif
	(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#ifdef WIZARD
	if(wizard) (void) signal(SIGQUIT,SIG_DFL);
#endif
	if(wt) {
		raw_print("");
		wait_synch();
	}
	resume_nhwindows();
	return(0);
}
#endif

#ifdef GETRES_SUPPORT

extern int FDECL(nh_getresuid, (uid_t *, uid_t *, uid_t *));
extern uid_t NDECL(nh_getuid);
extern uid_t NDECL(nh_geteuid);
extern int FDECL(nh_getresgid, (gid_t *, gid_t *, gid_t *));
extern gid_t NDECL(nh_getgid);
extern gid_t NDECL(nh_getegid);

int
(getresuid)(ruid, euid, suid)
uid_t *ruid, *euid, *suid;
{
    return nh_getresuid(ruid, euid, suid);
}

uid_t
(getuid)()
{
    return nh_getuid();
}

uid_t
(geteuid)()
{
    return nh_geteuid();
}

int
(getresgid)(rgid, egid, sgid)
gid_t *rgid, *egid, *sgid;
{
    return nh_getresgid(rgid, egid, sgid);
}

gid_t
(getgid)()
{
    return nh_getgid();
}

gid_t
(getegid)()
{
    return nh_getegid();
}

#endif	/* GETRES_SUPPORT */
