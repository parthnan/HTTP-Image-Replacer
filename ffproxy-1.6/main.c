/*
 * ffproxy (c) 2002-2005 Niklas Olmes <niklas@noxa.de>
 * http://faith.eu.org
 * 
 * $Id: main.c,v 2.2 2005/01/05 15:12:49 niklas Exp niklas $
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

#include <stdio.h>

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#include <string.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <time.h>
#include <pwd.h>
#include <grp.h>

#include "cfg.h"
#include "print.h"
#include "socket.h"
#include "db.h"
#include "dns.h"
#include "signals.h"

#if !defined(HAVE_DAEMON) || defined(NEED_DAEMON) || defined(__sun__)
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

static int daemon(int, int);
int
daemon(int nochdir, int noclose)
{
	int f;
	f = open("/dev/null", O_RDWR);
	(void) dup2(STDIN_FILENO, f);
	(void) dup2(STDERR_FILENO, f);
	(void) dup2(STDOUT_FILENO, f);
	if (fork())
		_exit(0);
	return 0;
}
#endif

static void     usage(void);
static void     drop_privileges(void);

static const char version[] = "1.6";
static const char rcsid[] = "$Id: main.c,v 2.2 2005/01/05 15:12:49 niklas Exp niklas $";
char            loop_header[100];

struct cfg      config;

int
main(int argc, char *argv[]) //takes in command line arguments and their no
{
	int             c, nowarn;
	char		*prgname;

	prgname = argv[0];
	nowarn = 0;

	(void) memset(&config, 0, sizeof(config));  //void *memset(void *str, int c, size_t n) copies the character c to the first n characters of the string pointed to, by the argument str.
	*config.ipv4 = '\0';
	*config.ipv6 = '\0';
	config.port = 0;
	config.daemon = 0;
	config.childs = 10;
	config.ccount = 0;
	config.backlog = 4;
	config.uid = 0L;
	config.gid = 0L;
	*config.chroot = '\0';
	(void) strncpy(config.dbdir, DATADIR, sizeof(config.dbdir) - 1);
	config.dbdir[sizeof(config.dbdir) - 1] = '\0';
	(void) strncpy(config.file, CFGFILE, sizeof(config.file) - 1);
	config.file[sizeof(config.file) - 1] = '\0';
	*config.proxyhost = '\0';
	config.proxyport = 0;
	config.syslog = 1;
	config.logrequests = 0;
	config.use_ipv6 = 1;
	config.aux_proxy_ipv6 = 1;
	config.bind_ipv6 = 1;
	config.bind_ipv4 = 1;
	config.accel = 0;
	config.accelusrhost = 1;
	*config.accelhost = '\0';
	config.accelport = 80;
	config.kalive = 1;
	config.unr_con = 0;
	config.to_con = 5;
	config.first = 1;

	while ((c = getopt(argc, argv, "vdbBc:C:p:x:X:l:u:g:r:D:F:f:s4a:A:h")) != -1) {  //If there are no more option characters, getopt() returns -1.
	/* VVIMP : optstring is a string containing the legitimate option characters.
       If such a character is followed by a colon, the option requires an
       argument, so getopt() places a pointer to the following text in the
       same argv-element, or the text of the following argv-element, in
       optarg.  Two colons mean an option takes an optional arg*/
		switch (c) {
		case 'v':
			(void) printf("ffproxy version %s, %s\n",
				      version, rcsid);
			exit(0);
			break;
		case 'd':
			config.daemon = 1;
			break;
		case 'b':
			config.bind_ipv4 = 0;
			break;
		case 'B':
			config.bind_ipv6 = 0;
			break;
		case 'c':
			(void) strncpy(config.ipv4, optarg, sizeof(config.ipv4) - 1);
			config.ipv4[sizeof(config.ipv4) - 1] = '\0';
			break;
		case 'C':
			(void) strncpy(config.ipv6, optarg, sizeof(config.ipv6) - 1);
			config.ipv6[sizeof(config.ipv6) - 1] = '\0';
			break;
		case 'p':
			config.port = atoi(optarg);
			if (config.port > MAX_PORTS || !config.port) {
				(void) fprintf(stderr, "Invalid port number (-p %s)\n", optarg);
				exit(1);
			}
			break;
		case 'x':
			if (strlen(optarg) > sizeof(config.proxyhost) - 1 ) {
				(void) fprintf(stderr, "Proxy host name too long\n");
				exit(1);
			}
			(void) strncpy(config.proxyhost, optarg, sizeof(config.proxyhost) - 1);
			config.proxyhost[sizeof(config.proxyhost) - 1] = '\0';
			break;
		case 'X':
			config.proxyport = atoi(optarg);
			if (config.proxyport > MAX_PORTS || !config.proxyport) {
				(void) fprintf(stderr, "Invalid port number (-X %s)\n", optarg);
				exit(1);
			}
			break;
		case 'l':
			config.childs = atoi(optarg);
			if (!config.childs || config.childs > MAX_CHILDS) {
				(void) fprintf(stderr, "Invalid limit of child processes (-l %s)\n", optarg);
				exit(1);
			}
			break;
		case 'u':
			if (!(config.uid = atoi(optarg))) {
				struct passwd *pwd;
				if ((pwd = getpwnam(optarg)))         //returns a pointer to a structure containing the broken-out fields of the record in the password database (e.g., the local password file /etc/passwd, NIS, and LDAP) that matches the username name.
					config.uid = (unsigned long) pwd->pw_uid;
				else {
					(void) fprintf(stderr, "UID %s not found\n", optarg);
					exit(1);
				}
			}
			break;
		case 'g':
			if (!(config.gid = atoi(optarg))) {
				struct group *grp;
				if ((grp = getgrnam(optarg)))
					config.gid = (unsigned long) grp->gr_gid;
				else {
					(void) fprintf(stderr, "GID %s not found\n", optarg);
					exit(1);
				}
			}
			break;
		case 'r':
			if (strlen(optarg) > sizeof(config.chroot) - 1 ) {
				(void) fprintf(stderr, "chroot directory too long\n");
				exit(1);
			}
			(void) strncpy(config.chroot, optarg, sizeof(config.chroot) - 1);
			config.chroot[sizeof(config.chroot) - 1] = '\0';
			break;
		case 'D':
			if (strlen(optarg) > sizeof(config.dbdir) - 1 ) {
				(void) fprintf(stderr, "dbdir directory too long\n");
				exit(1);
			}
			(void) strncpy(config.dbdir, optarg, sizeof(config.dbdir) - 1);
			config.dbdir[sizeof(config.dbdir) - 1] = '\0';
			break;
		case 'F':
			nowarn = 1;
		case 'f':
			if (strlen(optarg) > sizeof(config.file) - 1 ) {
				(void) fprintf(stderr, "config file name too long\n");
				exit(1);
			}
			(void) strncpy(config.file, optarg, sizeof(config.file) - 1);
			config.file[sizeof(config.file) - 1] = '\0';
			if (*config.file && !nowarn && strcmp(config.file, "/dev/null") != 0)
				(void) fprintf(stdout, "Using config file (%s).\nPlease note that due to design, config file overwrites command line options.\nUse -F instead of -f to omit this warning message.\n", config.file);
			break;
		case 's':
			config.syslog = 0;
			break;
		case '4':
			config.use_ipv6 = 0;
			break;
		case 'a':
			(void) strncpy(config.accelhost, optarg, sizeof(config.accelhost) - 1);
			config.accelhost[sizeof(config.accelhost) - 1] = '\0';
			break;
		case 'A':
			config.accelport = atoi(optarg);
			break;
		case 'h':
			usage();
			/* NOTREACHED */
			break;
		default:
			(void) fprintf(stderr, "Error, type `%s -h' for help on usage\n", prgname);
			exit(1);
			break;
		}
	}
	//hereon, check for leftover args
	argc -= optind;            //optind is the index of the next element to be processed in argv,initialized to 1
	argv += optind;    //argv is pointer to array of arguments so +1!        //adjusts for a variable number of arguments eaten by getopt 

	if (*argv) {
		(void) fprintf(stderr, "Unknown argument left (%s)\nType `%s -h' for usage\n", *argv, prgname);
		exit(1);
	}

	setup_log_master();         //if (config.syslog)    {openlog("FFPROXY(master)", LOG_PID, 0);  }
	                           //openlog : void openlog (const char *ident, int option, int facility)
							   /*openlog opens or reopens a connection to Syslog in preparation for submitting messages.
							   ident is an arbitrary identification string which future syslog invocations will prefix to each message. This is intended to identify the source of the message*/
	info("started, initializing");
	load_databases();        //see db.c easy
	(void) resolve("localhost");        //dns.c : returns ip address in a number of host address(here localhost), looks up hostent type info on localhost
	drop_privileges();        //optind is the index of the next element to be processed in argv,initialized to 1

	if (config.daemon) {
		FILE           *fp;

		if (daemon(1, 0) != 0)
			fatal("daemon() failed");
		(void) close(0);
		(void) close(1);
		(void) close(2);

		(void) chdir(config.dbdir);
		if ((fp = fopen("ffproxy.pid", "w")) == NULL)
			fatal("cannot create pid file ffproxy.pid in %s", config.dbdir);

		(void) fprintf(fp, "%ld", (long) getpid());
		(void) fclose(fp);
	}
	(void) snprintf(loop_header, sizeof(loop_header), "X-Loop-%d-%d: true", getpid(), (int) time(NULL));

	init_sighandlers();
	open_socket();

	/* NOTREACHED */
	return 0;
}

static void
usage(void)
{
	(void) fprintf(stderr, "ffproxy %s -- (c) 2002-2005 Niklas Olmes <niklas@noxa.de>\n", version);
	(void) fprintf(stderr, "   GNU GPL.  Website: http://faith.eu.org/programs.html\n");
	(void) fprintf(stderr,
		       "usage: ffproxy [-vhds4bB] [-c host|ip] [-C host|ip] [-p port]\n"
                       "       [-x host|ip -X port] [-l max] [-u uid|usr -g gid|grp] [-r dir]\n"
                       "       [-D dir] [-f file] [-a host|ip] [-A port]\n"
		       " -v  print version number       -h  usage (this screen)\n"
		       " -d  become daemon              -s  silent.  don't log to syslog.\n"
		       " -4  use IPv4 only.  don't try contacting via IPv6.\n"
		       " -b  do *not* bind to IPv4\n"
		       " -B  do *not* bind to IPv6\n"
		       " -c host|ip   bind to IPv4 address (default is any)\n");
	(void) fprintf(stderr,
		       " -C host|ip   bind to IPv6 address (default is any)\n"
		       " -p port      bind to port\n"
		       " -x host|ip   auxiliary forward proxy\n"
		       " -X port      auxiliary forward proxy port\n"
		       " -l max       maximum number of concurrent requests\n"
		       " -u uid|user  change uid\n"
		       " -g gid|group change gid\n"
		       " -r dir       chroot to dir\n"
		       " -D dir       databases are in dir (default is %s)\n"
		       " -f file      use config file (default is %s; *overwrites*)\n"
		       " -a host|ip   auxiliary forward server to use\n"
		       " -A port      auxiliary forward server port (default is 80)\n",
			DATADIR, CFGFILE);
	exit(1);
}

static void
drop_privileges(void)
{
	extern struct cfg config;
	struct passwd  *pwd;

	if (config.uid == 0 || config.gid == 0) {
		info("not changing UID/GID");
	} else {
		if ((pwd = getpwuid(config.uid)) == NULL)  //returns a pointer to a structure containing the broken-out fields of the record in the password database that matches the user ID uid.
			fatal("getpwuid() failed (non-existent UID entry?)");

		if (*config.chroot != '\0') {
			if (chdir(config.chroot) != 0) //The chdir() function shall cause the directory named by the pathname pointed to by the path argument to become the current working directory; 
			                               //that is, the starting point for path searches for pathnames not beginning with '/'.On success, zero is returned.  On error, -1 is returned
				fatal("chdir() failed");
			if (chroot(config.chroot) != 0)
				fatal("chroot() failed");
			info("chroot()ed to %s\n", config.chroot);
		}
		if (setgid(config.gid) != 0) //sets the effective group ID of the calling process.
			fatal("setgid() failed");
		if (initgroups(pwd->pw_name, config.gid) != 0)
			fatal("initgroups() failed");

		if (setuid(config.uid) != 0) //sets the effective user ID of the calling process. On success, zero is returned.  On error, -1 is returned, and errno is set appropriately.
			fatal("setuid() failed");

	}

	info("=> UID(%d), EUID(%d), GID(%d), EGID(%d)", getuid(), geteuid(), getgid(), getegid());
}
