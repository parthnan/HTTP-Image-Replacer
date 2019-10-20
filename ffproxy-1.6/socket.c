/*
 * ffproxy (c) 2002, 2003 Niklas Olmes <niklas@noxa.de>
 * http://faith.eu.org
 * 
 * $Id: socket.c,v 2.1 2004/12/31 08:59:15 niklas Exp $
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
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
/*
makes available a type, socklen_t, which is an unsigned opaque integral type of length of at least 32 bits. 

 sockaddr structure :

sa_family_t   sa_family       address family
char          sa_data[]       socket address (variable-length data)

functions in sys/socket.h 
int     accept(int socket, struct sockaddr *address,
             socklen_t *address_len);
int     bind(int socket, const struct sockaddr *address,
             socklen_t address_len);
int     connect(int socket, const struct sockaddr *address,
             socklen_t address_len);
int     getpeername(int socket, struct sockaddr *address,
             socklen_t *address_len);
int     getsockname(int socket, struct sockaddr *address,
             socklen_t *address_len);
int     getsockopt(int socket, int level, int option_name,
             void *option_value, socklen_t *option_len);
int     listen(int socket, int backlog);
ssize_t recv(int socket, void *buffer, size_t length, int flags);
ssize_t recvfrom(int socket, void *buffer, size_t length,
             int flags, struct sockaddr *address, socklen_t *address_len);
ssize_t recvmsg(int socket, struct msghdr *message, int flags);
ssize_t send(int socket, const void *message, size_t length, int flags);
ssize_t sendmsg(int socket, const struct msghdr *message, int flags);
ssize_t sendto(int socket, const void *message, size_t length, int flags,
             const struct sockaddr *dest_addr, socklen_t dest_len);
int     setsockopt(int socket, int level, int option_name,
             const void *option_value, socklen_t option_len);
int     shutdown(int socket, int how);
int     socket(int domain, int type, int protocol);
int     socketpair(int domain, int type, int protocol,
             int socket_vector[2]);
*/
#endif

#include <stdio.h>

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#include <string.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_NETDB_H
# include <netdb.h>
/*definitions for network database operations
hostent struct(not used)
 addrinfo structure :

int               ai_flags      Input flags. 
int               ai_family     Address family of socket. 
int               ai_socktype   Socket type. 
int               ai_protocol   Protocol of socket. 
socklen_t         ai_addrlen    Length of socket address. 
struct sockaddr  *ai_addr       Socket address of socket. 
char             *ai_canonname  Canonical name of service location. 
struct addrinfo  *ai_next       Pointer to next in list. 


*/
#endif
#include <poll.h>
/*
pollfd structure :

int         fd        the following descriptor being polled
short int   events    the input event flags (see below)
short int   revents   the output event flags (see below)

POLLIN
Same effect as POLLRDNORM | POLLRDBAND.
POLLRDNORM
Data on priority band 0 may be read.
POLLRDBAND
Data on priority bands greater than 0 may be read.
*/
#include "req.h"
#include "cfg.h"
#include "print.h"
#include "request.h"
#include "dns.h"
#include "access.h"
#include "socket.h"

#define DFLT_PORT	8080
#ifndef INFTIM
#define INFTIM	-1
#endif

void
open_socket(void)
{
	extern struct cfg config;    // cfg.h
	struct sockaddr claddr;     // see socket.h    (sa_family_t   sa_family       address family      char          sa_data[]       socket address (variable-length data))
	struct addrinfo hints[2], *res;  // see netdb.h lots of ints,one socklen_t,one sockaddr,pointer to next addrinfo in list
	struct clinfo  *clinfo;   //req.h : has name and ip strings( struct clinfo {char   name[128];char   ip[128];};)
	struct pollfd	s[2];      //input/output multiplexing poll.h has 3 ints lol
	//ignore s[1] as its only for ipv6; only s[0] used for ipv4
	socklen_t       claddr_len;  //unsigned opaque integral type of length of at least 32 bits. 
	pid_t           pid;     //process id type same as int, signed integer type.
	void           *foo;
	char		strport[6];
	char           *ip_add;
	int             st, cl, i;
	int		num_fd;
	int		isipv4;

	if (config.port == 0)
		config.port = DFLT_PORT;      //sets default port of not input in command line!
	(void) snprintf(strport, sizeof(strport), "%d", config.port);   //prints port no as string

	num_fd = 0;
	if (config.bind_ipv4)
		num_fd++;              //set num_fd to 1
	if (config.bind_ipv6)
		num_fd++;              //set num_fd to 2(ignore all ipv6)
		
	i = 0;
	(void) memset(s, 0, sizeof(s));      //set all 3ints in pollfd vars to 0
	s[0].fd = s[1].fd = 0;
	while (i < num_fd) {
		(void) memset(&hints[i], 0, sizeof(struct addrinfo));
		hints[i].ai_family = (i == 0 && config.bind_ipv4) ? PF_INET : PF_INET6;       //instead of regular AF_INET family
		hints[i].ai_socktype = SOCK_STREAM;                    //REPRESENTS TCP PROTOCOL
		hints[i].ai_flags = AI_PASSIVE;            
		if (i == 0 && config.bind_ipv4) {
			if (*config.ipv4 == '\0')
				ip_add = NULL;
			else
				ip_add = config.ipv4;
		} else {
			if (*config.ipv6 == '\0')
				ip_add = NULL;
			else
				ip_add = config.ipv6;
		}
		if (getaddrinfo(ip_add, strport, &hints[i], &res))           //
			fatal("getaddrinfo() failed for (%s) %s", ip_add, (i == 0 && config.bind_ipv4) ? "IPv4" : "IPv6");
		if ((s[i].fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {          //s[i].fd stores descriptor value for created sockets=>it is the integer socket value passed to all fns
			if (i == 1 || (config.ipv6 && !config.ipv4))
				fatal("socket() failed for IPv6, perhaps your system does not support IPv6.\nTry -B or `bind_ipv6 no' to disable IPv6 binding.\nError message");
			else
				fatal("socket() failed for IPv4");
		}
		if (setsockopt(s[i].fd, SOL_SOCKET, SO_REUSEADDR, &foo, sizeof(foo)) != 0) {
			/*set the option specified by the option_name argument, at the protocol level specified by the level argument, 
			to the value pointed to by the option_value argument for the socket associated with the file descriptor specified by the socket argument.
			
			SO_REUSEADDR
			Specifies that the rules used in validating addresses supplied to bind() should allow reuse of local addresses, if this is supported by the protocol. 
			This option takes an int value. This is a Boolean option.
			
			This socket option tells the kernel that even if this port is busy (in
            the TIME_WAIT state), go ahead and reuse it anyway*/
			(void) close(s[i].fd);
			fatal("setsockopt() failed for (%s) %s", ip_add, (i == 0 && config.bind_ipv4) ? "IPv4" : "IPv6");
		}
		//int descriptor value for socket, pointer to required sockaddr information of resource, length of address info of resource
		if (bind(s[i].fd,                       (struct sockaddr *) res->ai_addr,           res->ai_addrlen) < 0) {
			(void) close(s[i].fd);
#if defined(__linux__)
			if (i == 1)
				fatal("could not bind to IPv6, possibly because of\nLinux's ``feature'' to bind to IPv4 also.\nTry -b or binding to specific IPv6 address via -C\nif you're using IPv6 with Linux 2.4\nError message");
#endif /* __linux__ */
			fatal("bind() failed for (%s) %s", ip_add, (i == 0 && config.bind_ipv4) ? "IPv4" : "IPv6");
		}
		if (listen(s[i].fd, config.backlog) != 0) {
			(void) close(s[i].fd);
			fatal("listen() failed for (%s) %s",ip_add, (i == 0 && config.bind_ipv4) ? "IPv4" : "IPv6");
		}
		freeaddrinfo(res);
	
		s[i].events = POLLIN;
		i++;
	}
	
	if (config.bind_ipv4)
		info("waiting for requests on %s port %d (IPv4)", *config.ipv4 ? config.ipv4 : "(any)", config.port);
	if (config.bind_ipv6)
		info("waiting for requests on %s port %d (IPv6)", *config.ipv6 ? config.ipv6 : "(any)", config.port);

	claddr_len = sizeof(claddr);
	config.ccount = 0;
	cl = 0;
	isipv4 = config.bind_ipv4;

	for (;;) {     //INFINITE LOOP, is continueed if any error occours,stopped only by command line
		if (config.ccount >= config.childs) {
			(void) usleep(50000);     // cause the calling thread to be suspended from execution until either the number of realtime microseconds specified by the argument
			continue;
		}
		
		if (num_fd == 2) {                     //IGNORE ENTIRELY AS THIS IS IPV6
			i = poll(s, 2, INFTIM);
			if (i < 1) {
				continue;
			} else {
				if (s[0].revents == POLLIN) {
					st = s[0].fd;
					isipv4 = 1;
				} else {
					st = s[1].fd;
					isipv4 = 0;
				}
			}
		} 
		
		else
			st = s[0].fd;
		if ((cl = accept(st, (struct sockaddr *) & claddr, &claddr_len)) == -1) {    //THE ACTUAL ACCEPTANCE OF REQUEST ; 
		//cl STORES DESCRIPTOR OF CLIENT THAT IS USED AS FIRST ARGUMENT FOR  send() and recv()
			DEBUG(("open_socket() => accept() failed"));
			continue;
		}

		DEBUG(("open_socket() => connection, checking access"));
		clinfo = identify(&claddr, (socklen_t) isipv4 ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6));
		//identify or find out the client info(name and ip) of the client who sent claddr
		//clinfo has name and ip strings
		if (check_access(clinfo) != 0) {
			DEBUG(("open_socket() => no access"));
			if (config.logrequests)
				info("connection attempt from (%s) [%s], ACCESS DENIED", clinfo->name, clinfo->ip);
			free(clinfo);
			(void) close(cl);
			continue;
		}
		if (config.logrequests)
			info("connection attempt from (%s) [%s], access granted", clinfo->name, clinfo->ip);

		if ((pid = fork()) == -1) {
	//creates a new process, which is called child process, which runs concurrently with process (which process called system call fork) 
			DEBUG(("open_socket() => fork() failed"));
			free(clinfo);
			(void) close(cl);
			continue;
		} 
		else if (pid == 0) {
			(void) close(s[0].fd);
			if (num_fd == 2)
				(void) close(s[1].fd);
			setup_log_slave();
			handle_request(cl, clinfo);
			free(clinfo);
			(void) close(cl);
			exit(0);
		} 
		else {
			free(clinfo);
			config.ccount++;    //up the count of cycles (CYCLE COUNT)
			(void) close(cl);
		}
	}

	/* NOTREACHED */
}
