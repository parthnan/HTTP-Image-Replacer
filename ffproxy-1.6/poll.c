/*
 * ffproxy (c) 2002, 2003 Niklas Olmes <niklas@noxa.de>
 * http://faith.eu.org
 * 
 * $Id: poll.c,v 2.0 2004/06/08 06:39:51 niklas Exp $
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

#include <poll.h>

#include "poll.h"

int
my_poll(int s, int in)
{
	struct pollfd   p;

	p.fd = s;
	p.events = (in == IN) ? POLLIN : POLLOUT;

	switch (poll(&p, 1, 1000 * 20)) {
	case 0:
		return 0;
		break;
	case -1:
		return -1;
		break;
	default:
		return 1;
		break;
	}
}
