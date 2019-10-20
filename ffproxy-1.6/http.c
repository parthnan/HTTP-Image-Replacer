/*
 * ffproxy (c) 2002-2004 Niklas Olmes <niklas@noxa.de>
 * http://faith.eu.org
 * 
 * $Id: http.c,v 2.1 2004/12/31 08:59:15 niklas Exp $
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
#include <ctype.h>

#include "req.h"
#include "print.h"
#include "cfg.h"
#include "number.h"
#include "http.h"

#define my_isblank(c) ((c) == ' ' || (c) == '\t')

static const char http_get[] = "GET ";
static const char http_post[] = "POST ";
static const char http_head[] = "HEAD ";
static const char http_connect[] = "CONNECT ";
static const char http[] = "http://";
static const char httpv[] = "HTTP/";

int
http_url(struct req * r, const char *s)
{
	extern struct cfg config;
	size_t          i, k;
	char           *p;

	if (strncmp(http_get, s, strlen(http_get)) == 0) {
		r->type = GET;
		s += strlen(http_get);
	} else if (strncmp(http_post, s, strlen(http_post)) == 0) {
		r->type = POST;
		s += strlen(http_post);
	} else if (strncmp(http_head, s, strlen(http_head)) == 0) {
		r->type = HEAD;
		s += strlen(http_head);
	} else if (strncmp(http_connect, s, strlen(http_connect)) == 0) {
		r->type = CONNECT;
		s += strlen(http_connect);
	} else {
		r->type = UNKNOWN;
		return -1;
	}

	while (*s == ' ')
		s++;

	DEBUG(("http_url() => got url part (%s)", s));

	i = 0;
	if (config.accel) {
		r->relative = 0;
		DEBUG(("http_url() => using as accelerator proxy"));
		DEBUG(("http_url() => accelhost (%s) port %d", config.accelhost, config.accelport));
		i = snprintf(r->url, sizeof(r->url), "%s%s:%d", http, config.accelhost, config.accelport);
		if (i < 1)
			fatal_n("http_url() => accelhost is too long, can't create r->url");
		DEBUG(("http_url() => created url (%s) length (%d)", r->url, i));
	} else if (strncmp(s, http, strlen(http)) != 0) {
		r->relative = 1;
	} else {
		r->relative = 0;

		while (i < strlen(http)) {
			r->url[i] = http[i];
			i++, s++;
		}

		while (i < sizeof(r->url) - 1 && *s != '_'
		       && (isalnum((int) *s) || *s == '-' || *s == '.' || *s == ':'))
			r->url[i++] = tolower((int) *(s++));
		r->url[i] = '\0';
		if (*s != '/' && *s != ' ') {
			r->type = UNKNOWN;
			return -1;
		}
	}

	if (config.accel && strncmp(s, http, strlen(http)) == 0) {
		r->url[i++] = '\0';
		s += strlen(http);
		while (*s != ' ' && *s != '/' && *s != '\0' && isprint((int) *s))
			s++;
	}
	k = 0;
	while (i < sizeof(r->url) - 1 && k < sizeof(r->urlpath) - 1 && *s != ' ' && *s != '\0' && isprint((int) *s)) {
		r->urlpath[k++] = *s;
		r->url[i++] = *(s++);
	}
	if (k == 0)
		r->urlpath[k++] = '/';
	r->urlpath[k] = '\0';
	r->url[i] = '\0';
	if (*s != ' ') {
		r->type = UNKNOWN;
		return -1;
	}

	if (r->type == CONNECT)
		*r->url = '\0';

	DEBUG(("http_url() => extracted urlpath (%s)", r->urlpath));
	DEBUG(("http_url() => extracted url (%s)", r->url));

	while (*s == ' ')
		s++;

	DEBUG(("http_url() => got version part (%s)", s));

	if (strncasecmp(s, httpv, strlen(httpv)) != 0)
		return -1;
	s += strlen(httpv);
	r->vmajor = 0;
	r->vminor = 0;
	i = 0;
	while (i < 2 && *s != '\0' && isdigit((int) *s)) {
		r->vmajor = r->vmajor * 10 + my_ctoi(*(s++));
		i++;
	}
	if (*s == '.') {
		s++;
		while (i < 4 && *s != '\0' && isdigit((int) *s)) {
			r->vminor = r->vminor * 10 + my_ctoi(*(s++));
			i++;
		}
	}
	DEBUG(("http_url() => got type %d url (%s) version maj %d min %d",
	      r->type, r->url, r->vmajor, r->vminor));

	p = r->url;
	p += strlen(http);

	if(r->relative || r->type == CONNECT)
		return 0;

	i = 0;
	while ((isalnum((int) *p) || *p == '-' || *p == '.') && i < sizeof(r->host) - 1)
		r->host[i++] = tolower((int) *(p++));
	r->host[i] = '\0';

	if (i >= sizeof(r->host) - 1) {
		DEBUG(("http_url() => host: too long (%s)", r->host));
		*r->host = '\0';
		r->port = 0;
		return -1;
	}
	if (*p == ':') {
		p++;
		r->port = 0;
		while (isdigit((int) *p)) {
			r->port = r->port * 10 + my_ctoi(*(p++));
			if (r->port >= 65534) {
				DEBUG(("http_url() => port: bad port number"));
				r->port = 0;
				return -1;
			}
		}
		if (*p != '\0' && *p != ' ' && *p != '/') {
			DEBUG(("http_url() => port: bad port"));
			r->port = 0;
			return -1;
		}
		DEBUG(("http_url() => port: %d", r->port));
	} else {
		DEBUG(("http_url() => default port 80"));
		r->port = 80;
	}

	return 0;
}

static const char h_host[] = "Host: ";

int
http_rel(struct req * r, const char *s)
{
	size_t i;

	i = 0;
	if (r->relative && strncmp(s, h_host, strlen(h_host)) == 0) {
		r->relative = 0;

		s += strlen(h_host);
		while (*s == ' ')
			s++;
		while ((isalnum((int) *s) || *s == '-' || *s == '.') && i < sizeof(r->host) - 1)
			r->host[i++] = tolower((int) *(s++));
		r->host[i] = '\0';
		if (i >= sizeof(r->host) || (*s != ':' && *s != '\0')) {
			DEBUG(("http_rel() => invalid host header (%s)", r->host));
			r->host[0] = '\0';
			return 1;
		}
		DEBUG(("http_rel() => extracted host (%s)", r->host));
		if (*s == ':') {
			i = 0;
			r->port = 0;
			s++;
			while (isdigit((int) *s) && i++ < 6)
				r->port = r->port * 10 + my_ctoi(*(s++));
			if (i > 5 && i == 0) {
				DEBUG(("http_rel() => bad port number"));
				r->port = 0;
				return 1;
			}
			DEBUG(("http_rel() => extracted port %d", r->port));
		} else {
			if (r->type == CONNECT)
				r->port = 443;
			else
				r->port = 80;
		}
		if (strlen(r->url) + strlen(http) + strlen(r->host) + 7 >= sizeof(r->url)) {
			DEBUG(("http_rel() => URL will get too long"));
			return 1;
		} else {
			char o_url[sizeof(r->url)];
			(void) strncpy(o_url, r->url, sizeof(o_url) - 1);
			o_url[sizeof(o_url) - 1] = '\0';
			if (r->port == 80)
				(void) snprintf(r->url, sizeof(r->url), "http://%s%s", r->host, o_url);
			else
				(void) snprintf(r->url, sizeof(r->url), "http://%s:%d%s", r->host, r->port, o_url);
			DEBUG(("http_rel() => extracted URL (%s)", r->url));
		}
	}

	return 0;
}

static const char http_clen[] = "Content-Length: ";
static const char http_pkalive[] = "Proxy-Connection: keep-alive";
static const char http_kalive[] = "Connection: keep-alive";
#ifdef TSTAMP
static const char http_tstamp[] = "Last-Modified: ";
#endif

int
http_parse(struct req * r, const char *s)
{
	size_t          i;

	if (strncasecmp(http_clen, s, strlen(http_clen)) == 0) {
		DEBUG(("http_parse() => found clen header (%s)", s));

		s += strlen(http_clen);

		while (my_isblank(*s))
			s++;

		if (!isdigit((int) *s)) {
			DEBUG(("http_parse() => clen: no digit found (%s)", s));
			return -1;
		}
		r->clen = 0L;
		i = 0;
		while (i < 10 && isdigit((int) *s))
			r->clen = r->clen * 10L + (long) my_ctoi(*(s++));

		if (*s != '\0') {
			r->clen = 0L;
			DEBUG(("http_parse() => clen: too long"));
			return -1;
		}
		DEBUG(("http_parse() => clen: %ld bytes", r->clen));
		return 0;
#ifdef TSTAMP
	} else if (strncasecmp(http_tstamp, s, strlen(http_tstamp)) == 0) {
		DEBUG(("http_parse() => found tstamp header (%s)", s));

		s += strlen(http_tstamp);

		while (my_isblank(*s))
			s++;

		i = 0;
		while (i < sizeof(r->tstamp) - 1 && *s != '\0')
			r->tstamp[i++] = *(s++);
		r->tstamp[i] = '\0';

		if (*s != '\0') {
			r->tstamp[0] = '\0';
			DEBUG(("http_parse()_ => tstamp: too long"));
			return -1;
		}
		DEBUG(("http_parse() => tstamp: extracted (%s)", r->tstamp));
		return 0;
#endif
	} else if (strncasecmp(http_pkalive, s, strlen(http_pkalive)) == 0
		|| strncasecmp(http_kalive, s, strlen(http_kalive)) == 0) {
		DEBUG(("http_parse() => keep alive header found"));
		r->kalive = 1;
		return 1;
	}
	return 1;
}
