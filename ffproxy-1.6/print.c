/*
 * ffproxy (c) 2002, 2003 Niklas Olmes <niklas@noxa.de>
 * http://faith.eu.org
 * 
 * $Id: print.c,v 2.1 2004/12/31 08:59:15 niklas Exp $
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

#include <stdio.h>
#include <stdarg.h>

#include "configure.h"
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif

#include "cfg.h"
#include "print.h"

extern struct cfg config;

void
setup_log_master(void)
{
	if (config.syslog)
		openlog("FFPROXY(master)", LOG_PID, 0);
}

void
setup_log_slave(void)
{
	if (config.syslog)
		openlog("ffproxy(slave)", LOG_PID, 0);
}

void
fatal(const char *fmt,...)
{
	va_list         ap;
	char            buf[2048];

	va_start(ap, fmt);
	(void) vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (config.syslog)
		syslog(LOG_ERR, "%s, terminating\n", buf);

	perror(buf);
	exit(1);
}

void
fatal_n(const char *fmt,...)
{
	va_list         ap;
	char            buf[2048];

	va_start(ap, fmt);
	(void) vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (config.syslog)
		syslog(LOG_NOTICE, "%s, terminating\n", buf);

	(void) fprintf(stderr, "%s, terminating\n", buf);
	exit(1);
}

void
warn(const char *fmt,...)
{
	va_list         ap;
	char            buf[2048];
  /* dealing with variable arguments!
  int sum(int num_args, ...) 
  { 
   va_list ap;
   
   va_start(ap, num_args);
   for(i = 0; i < num_args; i++) {
      val += va_arg(ap, int);
   }
   va_end(ap);
  */
	va_start(ap, fmt);
	(void) vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (config.syslog)
		syslog(LOG_WARNING, "%s, continuing\n", buf);   //syslog outputs to a log file?

	(void) fprintf(stderr, "%s, continuing\n", buf);   //prints to standard error filehandle in command line
}

void
info(const char *fmt,...)
{
	va_list         ap;
	char            buf[2048];

	va_start(ap, fmt);
	(void) vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (config.syslog)
		syslog(LOG_INFO, "%s\n", buf);

#ifdef USE_DEBUG
	(void) fprintf(stdout, "%s\n", buf);
#endif
}
