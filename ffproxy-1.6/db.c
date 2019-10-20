/*
 * ffproxy (c) 2002-2004 Niklas Olmes <niklas@noxa.de>
 * http://faith.eu.org
 * 
 * $Id: db.c,v 2.1 2004/12/31 08:59:15 niklas Exp $
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
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <string.h>
#include <regex.h>
#include <pwd.h>
#include <grp.h>

#include "cfg.h"
#include "print.h"
#include "msg.h"
#include "alloc.h"
#include "file.h"
#include "db.h"

static void     clear_databases(void);
static void     clear_db(char *[]);
static void     clear_rdb(regex_t *[]);
static void     read_db(const char *, char *[]);
static void     read_rdb(const char *, regex_t *[]);
static void     read_file(const char *, struct msg *);
static void	read_config_file(void);
static void	verify_config(void);

#define MAX_E 256
regex_t        *a_ip[MAX_E];     //正規表現
regex_t        *a_host[MAX_E];
char           *a_dyndns[MAX_E];
regex_t        *f_host[MAX_E];
regex_t        *f_url[MAX_E];
regex_t        *f_hdr_drop[MAX_E];
regex_t        *f_hdr_match[MAX_E];
char           *f_hdr_entry[MAX_E];
char           *f_hdr_add[MAX_E];
regex_t        *f_rhdr_drop[MAX_E];
regex_t        *f_rhdr_match[MAX_E];
char           *f_rhdr_entry[MAX_E];
struct msg      e_inv;
struct msg      e_res;
struct msg      e_con;
struct msg      e_post;
struct msg      e_fil;


void
reload_databases(void)
{
	clear_databases();
	load_databases();
}

void
load_databases(void)
{
	extern struct cfg config;

	read_config_file();
	verify_config();

	if (*config.dbdir != '\0' && chdir(config.dbdir) != 0)
		fatal("could not chdir() to dbdir (%s)", config.dbdir);

	read_rdb("db/access.ip", a_ip);
	read_rdb("db/access.host", a_host);
	read_db("db/access.dyndns", a_dyndns);
	read_rdb("db/filter.host.match", f_host);
	read_rdb("db/filter.url.match", f_url);
	read_rdb("db/filter.header.drop", f_hdr_drop);
	read_rdb("db/filter.header.match", f_hdr_match);
	read_db("db/filter.header.entry", f_hdr_entry);
	read_db("db/filter.header.add", f_hdr_add);
	read_rdb("db/filter.rheader.drop", f_rhdr_drop);
	read_rdb("db/filter.rheader.match", f_rhdr_match);
	read_db("db/filter.rheader.entry", f_rhdr_entry);
	read_file("html/invalid", &e_inv);
	read_file("html/resolve", &e_res);
	read_file("html/connect", &e_con);
	read_file("html/post", &e_post);
	read_file("html/filtered", &e_fil);
}

static void
clear_databases(void)
{
	clear_rdb(a_ip);
	clear_rdb(a_host);
	clear_db(a_dyndns);
	clear_rdb(f_host);
	clear_rdb(f_url);
	clear_rdb(f_hdr_drop);
	clear_rdb(f_hdr_match);
	clear_db(f_hdr_entry);
	clear_db(f_hdr_add);
	clear_rdb(f_rhdr_drop);
	clear_rdb(f_rhdr_match);
	clear_db(f_rhdr_entry);
	free(e_inv.c);
	free(e_res.c);
	free(e_con.c);
	free(e_post.c);
	free(e_fil.c);
	e_inv.len = e_res.len = e_con.len = e_post.len = e_fil.len = 0;
}

static void
clear_db(char *db[])
{
	int             i;

	i = 0;
	while (db[i] != NULL) {
		free(db[i]);
		db[i++] = NULL;
	}
}

static void
clear_rdb(regex_t * r[])
{
	int             i;

	i = 0;
	while (r[i] != NULL) {
		regfree(r[i]);
		free(r[i]);
		r[i++] = NULL;
	}
}

static void
read_db(const char *f, char *db[])
{
	FILE           *fp;
	char            buf[512], *p;
	size_t          i;

	fp = my_fopen(f);

	i = 0;
	while (fgets(buf, sizeof(buf), fp) != NULL && i < MAX_E - 1) {
		if (buf[0] == '#' || buf[0] == '\r' || buf[0] == '\n')
			continue;
		if ((p = strchr(buf, '\n')) == NULL) {　　　//searches for the first occurrence of the character c (an unsigned char) in the string pointed to by the argument str.
			(void) fclose(fp);
			fatal_n("line too long in file %s", f);
		}
		*p = '\0';
		p = (char *) my_alloc(strlen(buf) + 1);
		strcpy(p, buf);
		db[i++] = p;
	}
	(void) fclose(fp);

	db[i] = NULL;
}

static void
read_rdb(const char *f, regex_t * r[])
{
	FILE           *fp;
	regex_t        *regex;
	char            buf[512], *p;
	char            errbuf[512];
	size_t          i;
	int             err;

	fp = my_fopen(f);

	i = 0;
	while (fgets(buf, sizeof(buf), fp) != NULL && i < MAX_E - 1) {
		if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r')
			continue;
		if ((p = strchr(buf, '\n')) == NULL) {
			(void) fclose(fp);
			fatal_n("line too long in file %s", f);
		}
		*p = '\0';
		regex = (regex_t *) my_alloc(sizeof(regex_t));
		if ((err = regcomp(regex, buf, REG_EXTENDED)) != 0) {       //compiles regular expression and stores result in regex. returns -1 if error.
			(void) regerror(err, regex, errbuf, sizeof(errbuf));
			warn("invalid regular expression (%s) in file (%s): %s", buf, f, errbuf);
			free(regex);
			continue;
		}
		r[i++] = regex;
	}
	(void) fclose(fp);

	r[i] = NULL;
}

static void
read_file(const char *fn, struct msg * m)
{
	int             f;
	char            buf[8192];
	ssize_t         len;

	f = my_open(fn);
	len = read(f, &buf, sizeof(buf));     //ssize_t read(int fildes, void *buf, size_t nbyte);
	//The read() function shall attempt to read nbyte bytes from the file associated with the open file descriptor, fildes, into the buffer pointed to by buf.

	m->c = (char *) my_alloc(len + 1);
	(void) memcpy(m->c, buf, len);
	m->c[len] = '\0';
	m->len = len;

	(void) close(f);
}

#include "dns.h"

static void
read_config_file(void)
{
	extern struct cfg config; 
	FILE           *fp;
	char            obuf[100];
	char            abuf[100];
	char            b[300];

	if (*config.file == '\0') {
		;
	} else if ((fp = fopen(config.file, "r")) != NULL) {
		while (fgets(b, sizeof(b), fp) != NULL) {
			(void) sscanf(b, "%99s %99s", obuf, abuf);
			if (config.first && strcmp("daemonize", obuf) == 0) {
				if (strcmp(abuf, "yes") == 0)
					config.daemon = 1;
				else
					config.daemon = 0;
				continue;
			} else if (strcmp("child_processes", obuf) == 0) {
				config.childs = atoi(abuf);
				continue;
			} else if (config.first && strcmp("bind_ipv4", obuf) == 0) {
				if (strcmp(abuf, "yes") == 0)
					config.bind_ipv4 = 1;
				else
					config.bind_ipv4 = 0;
				continue;
			} else if (config.first && strcmp("bind_ipv6", obuf) == 0) {
				if (strcmp(abuf, "yes") == 0)
					config.bind_ipv6 = 1;
				else
					config.bind_ipv6 = 0;
				continue;
			} else if (config.first && strcmp("bind_ipv4_host", obuf) == 0) {
				(void) strncpy(config.ipv4, abuf, sizeof(config.ipv4) - 1);
				config.ipv4[sizeof(config.ipv4) - 1] = '\0';
				continue;
			} else if (config.first && strcmp("bind_ipv6_host", obuf) == 0) {
				(void) strncpy(config.ipv6, abuf, sizeof(config.ipv6) - 1);
				config.ipv6[sizeof(config.ipv6) - 1] = '\0';
				continue;
			} else if (config.first && strcmp("port", obuf) == 0) {
				config.port = atoi(abuf);
				continue;
			} else if (strcmp("use_ipv6", obuf) == 0) {
				if (strcmp(abuf, "yes") == 0)
					config.use_ipv6 = 1;
				else
					config.use_ipv6 = 0;
				continue;
			} else if (config.first && strcmp("uid", obuf) == 0) {
				if (!(config.uid = atoi(abuf))) {
					struct passwd *pwd;
					if ((pwd = getpwnam(abuf)))
						config.uid = (unsigned long) pwd->pw_uid;
					else
						fatal_n("UID %s not found", abuf);
				}
				continue;
			} else if (config.first && strcmp("gid", obuf) == 0) {
				if (!(config.gid = atoi(abuf))) {
					struct group *grp;
					if ((grp = getgrnam(abuf)))
						config.gid = (unsigned long) grp->gr_gid;
					else
						fatal_n("GID %s not found", abuf);
				}
				continue;
			} else if (config.first && strcmp("chroot_dir", obuf) == 0) {
				(void) strncpy(config.chroot, abuf, sizeof(config.chroot) - 1);
				config.chroot[sizeof(config.chroot) - 1] = 0;
				continue;
			} else if (strcmp("forward_proxy", obuf) == 0) {
				(void) strncpy(config.proxyhost, abuf, sizeof(config.proxyhost) - 1);
				config.proxyhost[sizeof(config.proxyhost) - 1] = 0;
				continue;
			} else if (strcmp("forward_proxy_port", obuf) == 0) {
				config.proxyport = atoi(abuf);
				continue;
			} else if (strcmp("forward_proxy_ipv6", obuf) == 0) {
				if (strcmp(abuf, "yes") == 0)
					config.aux_proxy_ipv6 = 1;
				else
					config.aux_proxy_ipv6 = 0;
				continue;
			} else if (config.first && strcmp("db_files_path", obuf) == 0) {
				(void) strncpy(config.dbdir, abuf, sizeof(config.dbdir) - 1);
				config.dbdir[sizeof(config.dbdir) - 1] = 0;
				continue;
			} else if (strcmp("backlog_size", obuf) == 0) {
				config.backlog = atoi(abuf);
				continue;
			} else if (strcmp("use_syslog", obuf) == 0) {
				if (strcmp(abuf, "yes") == 0)
					config.syslog = 1;
				else
					config.syslog = 0;
				continue;
			} else if (strcmp("log_all_requests", obuf) == 0) {
				if (strcmp(abuf, "yes") == 0)
					config.logrequests = 1;
				else
					config.logrequests = 0;
				continue;
			} else if (strcmp("accel_host", obuf) == 0) {
				(void) strncpy(config.accelhost, abuf, sizeof(config.accelhost) - 1);
				config.accelhost[sizeof(config.accelhost) - 1] = '\0';
				continue;
			} else if (strcmp("accel_port", obuf) == 0) {
				config.accelport = atoi(abuf);
				continue;
			} else if (strcmp("accel_user_host", obuf) == 0) {
				if (strcmp(abuf, "yes") == 0)
					config.accelusrhost = 1;
				else
					config.accelusrhost = 0;
				continue;
			} else if (strcmp("use_keep_alive", obuf) == 0) {
				if (strcmp(abuf, "yes") == 0)
					config.kalive = 1;
				else
					config.kalive = 0;
				continue;
			} else if (strcmp("unrestricted_connect", obuf) == 0) {
				if (strcmp(abuf, "yes") == 0)
					config.unr_con = 1;
				else
					config.unr_con = 0;
				continue;
			} else if (strcmp("timeout_connect", obuf) == 0) {
				config.to_con = atoi(abuf);
				continue;
			} else if (!config.first) {
				continue;
			} else if (*obuf != '#') {
				warn("unknown option in config file %s:  %s", config.file, obuf);
				continue;
			}
		}
		(void) fclose(fp);
	} else {
		if (strcmp(config.file, CFGFILE) == 0)
			info("default config file (%s) not available, not using config file", CFGFILE);
		else
			fatal("unable to open config file %s", config.file);
	}
	config.first = 0;
}

#define ZHWRONG(a, o, v)	if(a < 1 || a > v) fatal_n("%s is set < 1 or value is too high (maximum:  %d, current:  %d)", o, v, a);
#define HWRONG(a, o, v)		if(a < 0 || a > v) fatal_n("Value of %s is set too high or negative (maximum:  %d, current:  %d)", o, v, a);
#define HUWRONG(a, o, v)		if(a > v) fatal_n("Value of %s is set too high (maximum:  %d, current:  %d)", o, v, a);

static void
verify_config(void)
{
	extern struct cfg config;

	HUWRONG(config.port, "port", MAX_PORTS);
	ZHWRONG(config.childs, "child_processes", MAX_CHILDS);
	HWRONG(config.backlog, "backlog_size", MAX_BACKLOG);
	HUWRONG(config.proxyport, "forward_proxy_port", MAX_PORTS);
	HUWRONG(config.uid, "uid", MAX_UID);
	HUWRONG(config.gid, "gid", MAX_GID);
	HUWRONG(config.accelport, "accel_port", MAX_PORTS);

	if ((config.uid && !config.gid) || (!config.uid && config.gid))
		fatal_n("Only one of uid and gid is set to non-zero.\nYou have to use both or none of them");
	if (*config.accelhost && config.accelport)
		config.accel = 1;
	else
		config.accel = 0;
	if (!config.bind_ipv4 && !config.bind_ipv6)
		fatal_n("Both IPv4 and IPv6 binding disabled.  This makes no sense");
}
