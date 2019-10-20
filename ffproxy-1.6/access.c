/*
 * ffproxy (c) 2002, 2003 Niklas Olmes <niklas@noxa.de>
 * http://faith.eu.org
 * 
 * $Id: access.c,v 2.0 2004/06/08 06:39:51 niklas Exp $
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
#include <string.h>

#include "req.h"
#include "dbs.h"
#include "print.h"
#include "dns.h"
#include "regex.h"
#include "access.h"

int
check_access(const struct clinfo * host)
{
	int             i;

	if (*host->ip != '\0') {
		i = 0;
		while (a_ip[i] != NULL)
			if (do_regex(host->ip, a_ip[i++]) == 0)   //similar to regexec returns 0 if done
		//a_ shit is from regex.h ; a_ip possibly stores the result ip address of execution of regular expression, null is not wanted
				return 0;
        //return success if ip address found
		if (*host->name != '\0') {
			i = 0;
			while (a_host[i] != NULL)
				if (do_regex(host->name, a_host[i++]) == 0)
					return 0;
		}
		i = 0;
		while (a_dyndns[i] != NULL)
			if (strcmp(host->ip, resolve_to_a(a_dyndns[i++])) == 0)   //same as ip_to_a(resolve())
				return 0;
	}

	DEBUG(("check_access() => done, no access.  IP (%s) Host (%s)", host->ip, host->name));

	return 1;
}
