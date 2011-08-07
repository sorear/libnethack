/*	SCCS Id: @(#)unixtty.c	3.4	1990/22/02 */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* tty.c - (Unix) version */

/* With thanks to the people who sent code for SYSV - hpscdi!jon,
 * arnold@ucsf-cgl, wcs@bo95b, cbcephus!pds and others.
 */

#define NEED_VARARGS
#include "hack.h"

static void
setctty()
{
}

/*
 * Get initial state of terminal, set ospeed (for termcap routines)
 * and switch off tab expansion if necessary.
 * Called by startup() in termcap.c and after returning from ! or ^Z
 */
void
gettty()
{
}

/* reset terminal to original state */
void
settty(s)
const char *s;
{
	end_screen();
	if(s) raw_print(s);
}

void
setftty()
{
	iflags.cbreak = ON;
	iflags.echo = OFF;

	start_screen();
}

void
intron()		/* enable kbd interupts if enabled when game started */
{
}

void
introff()		/* disable kbd interrupts if required*/
{
}

void
error VA_DECL(const char *,s)
       VA_START(s);
       VA_INIT(s, const char *);
       char out[1024];
       Vsprintf(out, s,VA_ARGS);
       nh_output(out, strlen(out));
       (void) nh_output("\r\n", 2);
       VA_END();
       nh_exit(EXIT_FAILURE);
}
