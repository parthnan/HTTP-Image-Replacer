/*
 * ffproxy (c) 2002-2004 Niklas Olmes <niklas@noxa.de>
 * http://faith.eu.org
 * 
 * $Id: number.c,v 1.1 2004/12/31 09:36:09 niklas Exp $
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

#include "number.h"

static const char *my_numbers = "0123456789";
#define LEN 10

int my_ctoi(const char c)
{
	int value = 0;

	while(value < LEN && my_numbers[value] != c)
		value++;

	if(value == LEN)
		value = 0;

	return value;
}
