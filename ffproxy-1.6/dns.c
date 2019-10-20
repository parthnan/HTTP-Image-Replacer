/*
 * ffproxy (c) 2002, 2003 Niklas Olmes <niklas@noxa.de>
 * http://faith.eu.org
 * 
 * $Id: dns.c,v 2.1 2004/12/31 08:59:15 niklas Exp $
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
/*lot of xxx_t types are defined*/
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
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
/*
sockaddr_in structure :

sa_family_t     sin_family   AF_INET. 
in_port_t       sin_port     Port number. 
struct in_addr  sin_addr     IP address. 
*/
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
/*
in_port_t
An unsigned integral type of exactly 16 bits.
in_addr_t
An unsigned integral type of exactly 32 bits.

arpa/net functions
in_addr_t      inet_addr(const char *cp);
in_addr_t      inet_lnaof(struct in_addr in);
struct in_addr inet_makeaddr(in_addr_t net, in_addr_t lna);
in_addr_t      inet_netof(struct in_addr in);
in_addr_t      inet_network(const char *cp);
char          *inet_ntoa(struct in_addr in);

*/
#endif

#include <stdio.h>
#include <string.h>
#ifdef HAVE_NETDB_H
# include <netdb.h>
/*definitions for network database operations
hostent structure :

char   *h_name       Official name of the host. 
char  **h_aliases    A pointer to an array of pointers to 
                     alternative host names, terminated by a 
                     null pointer. 
int     h_addrtype   Address type. 
int     h_length     The length, in bytes, of the address. 
char  **h_addr_list  A pointer to an array of pointers to network 
                     addresses (in network byte order) for the host, 
                     terminated by a null pointer. 
*/
#endif

#include "req.h"
#include "alloc.h"
#include "print.h"
#include "dns.h"

static char	*ip_to_a(in_addr_t);

in_addr_t
resolve(const char *h)
{
	struct hostent *hp;
	in_addr_t	ip;   // same as 32bit int

	if ((ip = inet_addr(h)) != INADDR_NONE)   // convert the string pointed to by cp, in the standard IPv4 dotted decimal notation, to an integer value suitable for use as an Internet address.
		return ip;

	if ((hp = gethostbyname(h)) == NULL)    //returns information (hostent type) about the host named name. If the lookup fails, it returns a null pointer.
		return INADDR_NONE;
	else {
		(void) memcpy(&ip, hp->h_addr, hp->h_length);    //copy entire host address to ip variable
		//void *memcpy(void *str1, const void *str2, size_t n) copies n characters from memory area str2 to memory area str1.
		return ip; 
	}
}

static char    *
ip_to_a(in_addr_t ip)
{
	char           *p;
	struct in_addr  addr;

	addr.s_addr = ip;
	p = inet_ntoa(addr);

	return p;
}

char           *
resolve_to_a(const char *h)
{
	char           *p;

	p = ip_to_a(resolve(h));

	return p;
}

//called in socket.c ; is given the 2nd(socket address iinfo) and 3rd(length of socket address variable) arguments of connect()/bind() etc.
//returns name and ip address of client
struct clinfo  * identify(const struct sockaddr * addr, socklen_t salen)
{
	struct clinfo  *host;

	host = (struct clinfo *) my_alloc(sizeof(struct clinfo));
	(void) memset(host, 0, sizeof(struct clinfo));

	if (getnameinfo(addr, salen, host->name, sizeof(host->name), NULL, 0, NI_NAMEREQD))
		*host->name = '\0';
	DEBUG(("identify() => getnameinfo() for Reverse Name returned (%s)", host->name));

	if (getnameinfo(addr, salen, host->ip, sizeof(host->ip), NULL, 0, NI_NUMERICHOST))
		*host->ip = '\0';
	DEBUG(("identify() => getnameinfo() for IP Address returned (%s)", host->ip));
/*int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                  char *nodename, socklen_t nodenamelen, 
                  char *servname, socklen_t servnamelen, 
                  int flags);*/
 //translates a socket address to a node name and service location, all of which are defined as with getaddrinfo().
	return host;
}
