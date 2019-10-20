/*
 * ffproxy (c) 2002, 2003 Niklas Olmes <niklas@noxa.de>
 * http://faith.eu.org
 * 
 * $Id: filter.c,v 2.1 2004/12/31 08:59:15 niklas Exp $
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
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#include <string.h>

#include "req.h"
#include "dbs.h"
#include "print.h"
#include "alloc.h"
#include "http.h"
#include "regex.h"
#include "filter.h"

static void     rotate(int, char *[]);

extern char     loop_header[];

int
filter_request(struct req * r)
{
	size_t          i;
	int             j;

	i = 0;
	while (f_host[i] != NULL)
		if (do_regex(r->host, f_host[i++]) == 0)
			return 1;

	i = 0;
	while (f_url[i] != NULL)
		if (do_regex(r->url, f_url[i++]) == 0)
			return 1;

	i = 0;
start_over:
	while (r->header[i] != NULL && i < sizeof(r->header) - 2) {
		DEBUG(("filter_request() => header entry %d (%s)", i, r->header[i]));

		if (strncasecmp(r->header[i], loop_header, strlen(loop_header)) == 0) {
			DEBUG(("filter_request() => LOOP DETECTED"));
			r->loop = 1;
			return -1;
		}
		if (http_parse(r, r->header[i]) == 0)
			goto skip;

		j = 0;
		while (f_hdr_drop[j] != NULL)
			if (do_regex(r->header[i], f_hdr_drop[j++]) == 0)
				return 1;

		j = 0;
		while (f_hdr_entry[j] != NULL) {
			if (strncasecmp(r->header[i], f_hdr_entry[j], strlen(f_hdr_entry[j])) == 0) {
				rotate(i, r->header);
				goto start_over;
			}
			j++;
		}

		j = 0;
		while (f_hdr_match[j] != NULL) {
			if (do_regex(r->header[i], f_hdr_match[j]) == 0) {
				rotate(i, r->header);
				goto start_over;
			}
			j++;
		}

skip:
		i++;
	}

	if (r->header[i] != NULL)
		free(r->header[i]);

	r->header[i] = (char *) my_alloc(strlen(loop_header) + 1);
	(void) strcpy(r->header[i], loop_header);

	DEBUG(("filter_request() => added loop header[%d] (%s)", i, r->header[i]));

	i++;
	j = 0;
	while (f_hdr_add[j] != NULL && i < sizeof(r->header) - 1) {
		r->header[i] = (char *) my_alloc(strlen(f_hdr_add[j]) + 1);
		(void) strcpy(r->header[i], f_hdr_add[j]);

		DEBUG(("filter_request() => added header[%d] (%s)", i, r->header[i]));
		i++, j++;
	}
	r->header[i] = NULL;

	DEBUG(("filter_request() => done, request ok"));

	return 0;
}

static const char http_pkalive[] = "Proxy-Connection: keep-alive";
static const char http_kalive[] = "Connection: keep-alive";

int
filter_remote(struct req * r)
{
	size_t		i;
	int             j;

	i = 0;
start_over:
	while (r->header[i] != NULL) {
		DEBUG(("filter_remote() => remote header entry %d (%s)", i, r->header[i]));

		if (strncasecmp(r->header[i], loop_header, strlen(loop_header)) == 0) {
			DEBUG(("filter_request() => LOOP DETECTED"));
			r->loop = 1;
			return -1;
		}
		if (http_parse(r, r->header[i]) == 0)
			goto skip;

		j = 0;
		while (f_rhdr_drop[j] != NULL)
			if (do_regex(r->header[i], f_rhdr_drop[j++]) == 0)
				return 1;

		j = 0;
		while (f_rhdr_entry[j] != NULL) {
			if (strncasecmp(r->header[i], f_rhdr_entry[j], strlen(f_rhdr_entry[j])) == 0) {
				rotate(i, r->header);
				goto start_over;
			}
			j++;
		}

		j = 0;
		while (f_rhdr_match[j] != NULL) {
			if (do_regex(r->header[i], f_rhdr_match[j]) == 0) {
				rotate(i, r->header);
				goto start_over;
			}
			j++;
		}
skip:
		i++;

	}
	if(r->kalive && i - 2 < sizeof(r->header)) {
		r->header[i] = (char *) my_alloc(strlen(http_pkalive) + 1);
		(void) strcpy(r->header[i], http_pkalive);
		r->header[++i] = (char *) my_alloc(strlen(http_kalive) + 1);
		(void) strcpy(r->header[i], http_kalive);
		r->header[++i] = NULL;
	} else if(r->kalive) {
		r->kalive = 0;
	}

	DEBUG(("filter_remote() => done, request ok"));

	return 0;
}


static void
rotate(int i, char *a[])
{
	if (a[i] == NULL) {
		DEBUG(("rotate() => entry to rotate, %d, is NULL", i));
	} else {
		DEBUG(("rotate() => freeing a[%d] == (%s)", i, a[i]));
		free(a[i]);
		a[i] = a[i + 1];
		if (a[i + 1] != NULL) {
			i++;
			while (a[i] != NULL) {
				a[i] = a[i + 1];
				i++;
			}
		}
	}

	DEBUG(("rotate() => done"));
}
