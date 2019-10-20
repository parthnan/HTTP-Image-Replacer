#include "configure.h"
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif

#ifndef INADDR_NONE
# define INADDR_NONE	-1
#endif

in_addr_t	resolve(const char *);
char           *resolve_to_a(const char *);
struct clinfo  *identify(const struct sockaddr *, socklen_t);
