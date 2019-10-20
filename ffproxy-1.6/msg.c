/*
 * ffproxy (c) 2002, 2003 Niklas Olmes <niklas@noxa.de>
 * http://faith.eu.org
 * 
 * $Id: msg.c,v 2.1 2004/12/31 08:59:15 niklas Exp $
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
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "req.h"
#include "dbs.h"
#include "msg.h"
#include "poll.h"

void
err_msg(int s, struct req * r, int m)
{
	char            msg[8192];
	char           *p;
	size_t          i;
	int             j;

	p = NULL;

	switch (m) {
	case E_INV:
		p = e_inv;
		break;
	case E_RES:
		p = e_res;
		break;
	case E_CON:
		p = e_con;
		break;
	case E_POST:
		p = e_post;
		break;
	case E_FIL:
		p = e_fil;
		break;
	}

	*msg = '\0';
	i = 0;
	while (p != NULL && *p != '\0' && i < sizeof(msg) - 1)
		if (*p == '$') {
			switch (*(p + 1)) {
			case 'u':
				j = 0;
				while (i < sizeof(msg) - 1 && r->url[j] != '\0')
					msg[i++] = r->url[j++];
				p += 2;
				break;
			case 'h':
				j = 0;
				while (i < sizeof(msg) - 1 && r->host[j] != '\0')
					msg[i++] = r->host[j++];
				p += 2;
				break;
			case 'c':
				j = 0;
				while (i < sizeof(msg) - 1 && r->cl->name[j] != '\0')
					msg[i++] = r->cl->name[j++];
				p += 2;
				break;
			default:
				msg[i++] = *(p++);
				break;
			}
		} else
			msg[i++] = *(p++);

	msg[i] = '\0';

	if (i > 0 && my_poll(s, OUT))
		(void) write(s, msg, i - 1);
}
