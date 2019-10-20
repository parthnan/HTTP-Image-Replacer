/*
 * ffproxy (c) 2002, 2003 Niklas Olmes <niklas@noxa.de>
 * http://faith.eu.org
 * 
 * $Id: signals.c,v 2.1 2004/12/31 08:59:15 niklas Exp $
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675
 * Mass Ave, Cambridge, MA 02139, USA.
 */

#include "configure.h"
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
/*

*/
#endif

#include <signal.h>
/*
SIGCHLD     //represents int numbers!
I
Child process terminated, stopped,
SIGINT
T
Terminal interrupt signal.
SIGHUP
T
Hangup.
SIGTERM
T
Termination signal.
*/
#include "cfg.h"
#include "print.h"
#include "db.h"
#include "signals.h"
#include "configure.h"

static RETSIGTYPE sigchld(int);
static RETSIGTYPE sighup(int);
static RETSIGTYPE sigterm(int);
static RETSIGTYPE sigint(int);

extern struct cfg config;

static RETSIGTYPE
sigchld(int dummy)
{
	int             status;
	pid_t           pid;

	(void) dummy;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
		config.ccount--;
}

static RETSIGTYPE
sighup(int dummy)
{
	(void) dummy;
	info("SIGHUP received, reloading databases");
	reload_databases();
}

static RETSIGTYPE
sigterm(int dummy)
{
	(void) dummy;
	fatal_n("SIGTERM received");
}

static RETSIGTYPE
sigint(int dummy)
{
	(void) dummy;
	fatal_n("SIGINT received");
}

void
init_sighandlers(void)
{
	signal(SIGCHLD, sigchld);    //void (*signal(int sig, void (*func)(int)))(int) 
	     //sets a function to handle signal i.e. a signal handler with signal number sig.
	signal(SIGHUP, sighup);
	signal(SIGTERM, sigterm);
	signal(SIGINT, sigint);
}
