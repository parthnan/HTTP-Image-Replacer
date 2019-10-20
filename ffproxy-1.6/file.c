/*
 * ffproxy (c) 2002, 2003 Niklas Olmes <niklas@noxa.de>
 * http://faith.eu.org
 * 
 * $Id: file.c,v 2.1 2004/12/31 08:59:15 niklas Exp $
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

#include "configure.h"
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#include "print.h"
#include "file.h"

int
my_open(const char *path)
{
	int             f;

	if ((f = open(path, O_RDONLY, 0)) < 0)
		fatal("unable to open file %s", path);

	return f;
}

FILE           *
my_fopen(const char *path)
{
	FILE           *fp;

	if ((fp = fopen(path, "r")) == NULL)
		fatal("unable to open file %s", path);

	return fp;
}
