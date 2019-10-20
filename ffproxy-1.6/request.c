/*
 * ffproxy (c) 2002-2004 Niklas Olmes <niklas@noxa.de>
 * http://faith.eu.org
 * 
 * $Id: request.c,v 2.1 2004/12/31 08:59:15 niklas Exp $
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
#include <string.h>

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif 
#define original 0
//#define USE_DEBUG 0
#include <ctype.h>

#include "req.h"
#include "cfg.h"
#include "msg.h"
#include "alloc.h"
#include "print.h"
#include "http.h"
#include "filter.h"
#include "poll.h"
#include "request.h"
#include "configure.h"
int dafuq=0;
static int      read_header(int, struct req *);
static char     sgetc(int);
static size_t   getlines(int, char[], int);
static int      do_request(int, struct req *);

void
handle_request(int cl, struct clinfo * clinfo)
{
	extern struct cfg config;
	struct req      r;
	char            buf[2048];

keep_alive:
	(void) memset(&r, 0, sizeof(r));
	r.cl = clinfo;

	if (getlines(cl, buf, sizeof(buf)) < 1)
		*buf = '\0';

	if ((http_url(&r, buf)) == 0) {
		int             i;

		r.loop = 0;

		if (r.type == CONNECT) {
			DEBUG(("handle_request => CONNECT request detected"));
			if (read_header(cl, &r) != 0) {
				info("invalid CONNECT header from (%s) [%s]",
					clinfo->name, clinfo->ip);
				err_msg(cl, &r, E_INV);
			} else if (r.port != 443 && ! config.unr_con) {
				info("invalid CONNECT port (%d) for host (%s) from (%s) [%s]",
					r.port, r.host, clinfo->name, clinfo->ip);
				err_msg(cl, &r, E_INV);
			} else if (filter_request(&r) != 0) {
				info("filtered CONNECT request for (%s:%d) from (%s) [%s]",
					r.host, r.port, clinfo->name, clinfo->ip);
			} else {
				if (config.logrequests) {
					if (r.port == 443)
						info("HTTPS CONNECT to (%s) from (%s) [%s]",
							r.host, clinfo->name, clinfo->ip);
					else
						info("CONNECT to host (%s:%d) from (%s) [%s]",
							r.host, r.port, clinfo->ip);
				}
				i = do_request(cl, &r);
				switch (i) {
				case E_INV:
					info("invalid CONNECT request for (%s:%d) from (%s) [%s]", r.host, r.port, clinfo->name, clinfo->ip);
					break;
				case E_RES:
					info("resolve failure for host (%s) from (%s) [%s]", r.host, clinfo->name, clinfo->ip);
					break;
				case E_CON:
					info("connection failure for host (%s) from (%s) [%s]", r.host, clinfo->name, clinfo->ip);
					break;
				default:
					i = 0;
				}
				if (i != 0) {
					err_msg(cl, &r, i);
					r.kalive = 0;
				}
			}
			i = 0;
			while (r.header[i] != NULL)
				free(r.header[i++]);
			r.header[0] = NULL;
		} else if (read_header(cl, &r) != 0) {
			info("invalid header from (%s) [%s]", clinfo->name, clinfo->ip);
			err_msg(cl, &r, E_INV);

			i = 0;
			while (r.header[i] != NULL)
				free(r.header[i++]);
			r.header[0] = NULL;
		} else if (filter_request(&r) != 0) {
			info("filtered request for URL (%s) from (%s) [%s]", r.url, clinfo->name, clinfo->ip);
			if (r.loop)
				warn("LOOP DETECTED for URL (%s) from (%s) [%s]", r.url, clinfo->name, clinfo->ip);
			else
				err_msg(cl, &r, E_FIL);

			i = 0;
			while (r.header[i] != NULL)
				free(r.header[i++]);
			r.header[0] = NULL;
		} else {
			if (config.logrequests)
				info("request for URL (%s) from (%s) [%s]", r.url, clinfo->name, clinfo->ip);

			i = do_request(cl, &r);
			switch (i) {
			case E_INV:
				info("invalid request for URL (%s) from (%s) [%s]", r.url, clinfo->name, clinfo->ip);
				break;
			case E_RES:
				info("resolve failure for host (%s) from (%s) [%s]", r.host, clinfo->name, clinfo->ip);
				break;
			case E_CON:
				info("connection failure for host (%s) from (%s) [%s]", r.host, clinfo->name, clinfo->ip);
				break;
			case E_POST:
				info("failure while post for URL (%s) from (%s) [%s]", r.url, clinfo->name, clinfo->ip);
				break;
			case E_FIL:
				info("filtered request for URL (%s) from (%s) [%s]", r.url, clinfo->name, clinfo->ip);
				break;
			default:
				i = 0;
			}
			if (i != 0) {
				err_msg(cl, &r, i);
				r.kalive = 0;
			}

			i = 0;
			while (r.header[i] != NULL)
				free(r.header[i++]);
			r.header[0] = NULL;

			if (config.kalive && r.kalive && r.clen > 0L)
				goto keep_alive;
		}
	} else {
		if (*buf == '\0') {
			;
		} else {
			info("invalid request from (%s) [%s]", clinfo->name, clinfo->ip);
		}
	}
}

static int
read_header(int cl, struct req * r)
{
	size_t          len, i;
	char            buf[2048];
	char           *b, *p;

	i = 0;
	while ((len = getlines(cl, buf, sizeof(buf))) > 0 && i < sizeof(r->header) - 1) {
		b = buf;
		while (isspace((int) *b) && *(b++) != '\0');
		if (*b == '\0')
			continue;

		p = (char *) my_alloc(len + 1);
		(void) strcpy(p, b);
		r->header[i] = p;

		DEBUG(("read_header() => entry %d (%s)", i, r->header[i]));

		i++;
		
		if (r->relative && http_rel(r, p) != 0) {
			r->header[i] = NULL;
			return 1;
		}
	}
	r->header[i] = NULL;

	if (i >= sizeof(r->header) - 1)
		return 1;

	return 0;
}

static char
sgetc(int s)
{
	char            c;

	if (read(s, &c, 1) != 1 || c < 1)
		return -1;
	else
		return c;
}

static          size_t
getlines(int s, char buf[], int len)
{
	int             c;
	size_t          i;

	if (my_poll(s, IN) <= 0)
		return 0;

	i = 0;
	while (--len > 0) {
		c = sgetc(s);
		if (c == '\n' || c == -1)
			break;
		else if (c != '\r')
			buf[i++] = c;
	}
	buf[i] = '\0';

	return i;
}

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif
#include <stdio.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#include <string.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "dns.h"

static int
do_request(int cl, struct req * r)
{
	extern struct cfg config;
	unsigned long   ip;
	int             s;
	void           *foo;
	size_t          len, i;
	char            buf[4096];
	char            after[4096];
	char           *p;
	char           *op;
	char           *q;
	char           string[4096];
	char           *oafter;
	char           *newurl="https://images.pexels.com/photos/18495/pexels-photo.jpg?auto=compress&cs=tinysrgb&h=3X5";
	int             originallen=0;
	int             newurllen=strlen(newurl);
	int             difflen=0;
	size_t          beforelength;
	size_t          afterlength;
	size_t          digit;
	char           strbeforelen[20];
	char           strafterlen[20];
	int            ishtml=0; //htmlなら1,そうでなければ0
	int            srcnum=0;

	len = 0;
	ip = 0L;
	s = 0;

	if (config.use_ipv6 && (config.aux_proxy_ipv6 || *config.proxyhost == '\0')) {
		struct addrinfo hints, *res, *res0;
		char port[6];

		DEBUG(("do_request() => trying ipv6"));

		port[0] = '\0';
		(void) memset(&hints, 0, sizeof(hints));
		hints.ai_family = PF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		if (*config.proxyhost != '\0' && config.proxyport) {
			DEBUG(("do_request() => trying ipv6 for proxy %s port %d", config.proxyhost, config.proxyport));
			(void) snprintf(port, 6, "%d", config.proxyport);
			if (getaddrinfo(config.proxyhost, port, &hints, &res)) {
				DEBUG(("do_request() => getaddrinfo() failed for proxy %s", config.proxyhost));
				return E_RES;
			}
		} else {
			(void) snprintf(port, 6, "%d", r->port);
			if (getaddrinfo(r->host, port, &hints, &res)) {
				DEBUG(("do_request() => getaddrinfo() failed for %s", r->host));
				return E_RES;
			}
		}

		s = -1;
		for (res0 = res; res; res = res->ai_next) {
			if ((s = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
				continue;
			else if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
				(void) close(s);
				s = -1;
				continue;
			} else
				break;
		}
		freeaddrinfo(res0);

		if (s == -1) {
			if (*config.proxyhost != '\0' && config.proxyport) {
				DEBUG(("do_request() => socket() or connect() after getaddrinfo() failed for proxy %s port %d", config.proxyhost, config.proxyport));
			} else {
				DEBUG(("do_request() => socket() or connect() after getaddrinfo() failed for %s port %d", r->host, r->port));
			}
			return E_CON;
		}
	} else {
		struct sockaddr_in addr;

		DEBUG(("do_request() => not trying ipv6"));
		
		(void) memset(&addr, 0, sizeof(addr));

		if (*config.proxyhost != '\0' && config.proxyport) {
			DEBUG(("do_request() => using aux proxy w/o trying ipv6"));
			if ((addr.sin_addr.s_addr = resolve(config.proxyhost)) == INADDR_NONE) {
				DEBUG(("do_request() => resolve failure for proxy %s", config.proxyhost));
				return E_RES;
			}
			addr.sin_port = htons(config.proxyport);
			addr.sin_family = AF_INET;
		} else {
			if ((ip = resolve(r->host)) == INADDR_NONE) {
				DEBUG(("do_request() => resolve failure for %s", r->host));
				return E_RES;
			}
			addr.sin_addr.s_addr = ip;
			addr.sin_port = htons(r->port);
			addr.sin_family = AF_INET;
		}

		if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			DEBUG(("do_request() => socket() failed for %s port %d", r->host, r->port));
			return E_CON;
		} else if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &foo, sizeof(foo)) != 0) {
			DEBUG(("do_request() => setsockopt() failed for %s port %d", r->host, r->port));
			return E_CON;
		} else if (connect(s, (struct sockaddr *) & addr, sizeof(addr)) == -1) {
			DEBUG(("do_request() => connect() failed for %s port %d", r->host, r->port));
			return E_CON;
		}
	}

#ifdef USE_DEBUG
	i = 0;
	DEBUG(("do_request() => header is:"));
	while (r->header[i] != NULL)
		DEBUG(("=> [%s]", r->header[i++]));
#endif

	if (r->vmajor >= 1 && r->vminor >= 0)
		r->vmajor = 1, r->vminor = 0;

	if (config.accel && config.accelusrhost)
		len = snprintf(buf, sizeof(buf),
			       "%s %s HTTP/%d.%d\r\n",
			       ((r->type == GET) ? "GET"
				: ((r->type) == HEAD) ? "HEAD" : "POST"),
			       (*config.proxyhost && config.proxyport) != '\0' ? r->url :  r->urlpath,
			       r->vmajor, r->vminor);
	else if (r->port == 80)
		len = snprintf(buf, sizeof(buf),
			       "%s %s HTTP/%d.%d\r\n"
			       "Host: %s\r\n",
			       ((r->type == GET) ? "GET"
				: ((r->type) == HEAD) ? "HEAD" : "POST"),
			       (*config.proxyhost && config.proxyport) != '\0' ? r->url :  r->urlpath,
			       r->vmajor, r->vminor,
			       r->host);
	else if (r->port == 443 || r->type == CONNECT) {
		*buf = '\0';
		len = 0;
	} else
		len = snprintf(buf, sizeof(buf),
			       "%s %s HTTP/%d.%d\r\n"
			       "Host: %s:%d\r\n",
			       ((r->type == GET) ? "GET"
				: ((r->type) == HEAD) ? "HEAD" : "POST"),
			       (*config.proxyhost && config.proxyport) != '\0' ? r->url :  r->urlpath,
			       r->vmajor, r->vminor,
			       r->host, r->port);

	if (r->type != CONNECT) {
		i = 0;
		while (r->header[i] != NULL) {
			len += strlen(r->header[i]) + strlen("\r\n");
			if (len < sizeof(buf)) {
				(void) strncat(buf, r->header[i++], len);
				(void) strncat(buf, "\r\n", strlen("\r\n"));
			} else {
				DEBUG(("do_request() => header too big"));
				(void) close(s);
				i = 0;
				while (r->header[i] != NULL)
					free(r->header[i++]);
					r->header[0] = NULL;
				return E_INV;
			}
		}
	}
	i = 0;
	while (r->header[i] != NULL)
		free(r->header[i++]);
	r->header[0] = NULL;

	if (r->type != CONNECT) {
		len += strlen("\r\n");
		if (len >= sizeof(buf) - 1) {
			DEBUG(("do_request() => header too big"));
			(void) close(s);
			return E_INV;
		}
		(void) strncat(buf, "\r\n", strlen("\r\n"));

		DEBUG(("do_request() => request ready: type %d url (%s) host (%s) port %d",
		      r->type, r->url, r->host, r->port));
		DEBUG(("=> version maj %d min %d", r->vmajor, r->vminor));
		DEBUG(("=> header: (%s)", buf));
				//fprintf(stdout,"%s\n",buf);
		if (my_poll(s, OUT) <= 0 || write(s, buf, len) < 1) {
			DEBUG(("do_request() => sending request failed"));
			(void) close(s);
			return E_CON;
		}
	}
	if (r->type == POST) {
		long            rest;

		DEBUG(("do_request() => posting data"));

		if ((rest = r->clen) < 0L) {
			DEBUG(("do_request() => post: invalid clen %ld", r->clen));
			(void) close(s);
			return E_POST;
		}
		while (rest > 0L) {
			if (my_poll(cl, OUT) <= 0) {
				(void) close(s);
				return E_POST;
			}
			len = read(cl, buf, sizeof(buf));
			if (len < 1)
				break;
			else
				rest -= len;

			if (my_poll(s, OUT) <= 0 || write(s, buf, len) < 1) {
				DEBUG(("do_request() => post: error writing post data"));
				(void) close(s);
				return E_POST;
			}
		}
		DEBUG(("do_request() => post done"));
	}
	if (r->type != CONNECT) {
		i = 0;
		while ((len = getlines(s, buf, sizeof(buf))) > 0 && i < sizeof(r->header) - 1) {
			DEBUG(("do_request() => got remote header line: (%s)", buf));
			r->header[i] = (char *) my_alloc(len + 1);
			(void) strcpy(r->header[i++], buf);
		}
		r->header[i] = NULL;

		if (len > 0) {
			DEBUG(("do_request() => remote header too big"));
			(void) close(s);
			i = 0;
			while (r->header[i] != NULL)
				free(r->header[i++]);
			r->header[0] = NULL;
			return E_FIL;
		}
		if (filter_remote(r) != 0) {
			DEBUG(("do_request() => response was filtered"));
			(void) close(s);
			i = 0;
			while (r->header[i] != NULL)
				free(r->header[i++]);
			r->header[0] = NULL;
			return E_FIL;
		}
		*buf = '\0';
		len = 0;
		i = 0;
		while (r->header[i] != NULL) {
			len += strlen(r->header[i]) + strlen("\r\n");
			if (len < sizeof(buf) - 1) {
				(void) strcat(buf, r->header[i++]);
				(void) strcat(buf, "\r\n");
			} else {
				DEBUG(("do_request() => remote header too big (at concatenation)"));
				i = 0;
				while (r->header[i] != NULL)
					free(r->header[i++]);
				r->header[0] = NULL;
				(void) close(s);
				return E_FIL;
			}
		}
		i = 0;
		while (r->header[i] != NULL)
			free(r->header[i++]);
		r->header[0] = NULL;

		len += strlen("\r\n");
		if (len >= sizeof(buf) - 1) {
			DEBUG(("do_request() => remote header too big (at final)"));
			(void) close(s);
			return E_FIL;
		}
		(void) strcat(buf, "\r\n");

		DEBUG(("do_request() => remote header ready: (%s)", buf));
                #ifdef original
                #else
 	        #endif
			//FILE* fp=fopen("file.txt","w");
                     //fprintf(stdout,"%s\n",buf);//fflush(stdout);
			//fclose(fp);
		printf("\n------------------------Original Header---------------------------\n");
		printf("header=%s\n",buf);
		if(strstr(buf,"text/html")!=NULL){
		  ishtml=1;
		  if((p=strstr(buf,"Content-Length"))!=NULL){
		    p+=16;
		    int i=0;
		    while(*(p+i)-48>=0 && *(p+i)-48<=9){
		      //printf("*p+%d=%c\n",i,*(p+		      //strbeforelen[i]=*p;
		      i++;
		      //i++;
		    }
		    strncpy(strbeforelen,p,i+1);
		    *p='\0';
		    p+=i;
		    strcpy(after,p);
		    //printf("strbeforelen=%s\n",strbeforelen);
		    beforelength=atoi(strbeforelen);
		    afterlength=beforelength+300;
		    //itoa
		    {
		      static const char table[]="0123456789";
		      size_t len2=afterlength;
		      char *head=strafterlen;
              	      char *tail=strafterlen;
	       	      char temp;
		      if(len2==0){
	                *tail++='0';
	 	      }
	      	      else for(head=tail;len2!=0;len2/=10){
	       	        *tail++=table[len2%10];
      		      }
	    	      *tail='\0';
	      	      for(tail--;head<tail;head++,tail--){
	      	        temp=*head;
	      	        *head=*tail;
	       	        *tail=temp;
	       	      }
		      }
		    //printf("beforelength=%s\n",strbeforelen);
		    //printf("afterlength=%s\n",strafterlen);
		    int digbefore=0;
		    int digafter=0;
		    int digdiff=0;
		    /*while(beforelength!=0){
		      beforelength/=10;
		      digbefore++;
		    }
		    while(afterlength!=0){
		      afterlength/=10;
		      digafter++;
		      }*/
		    digbefore=strlen(strbeforelen);
		    digafter=strlen(strafterlen);
		    digdiff=digafter-digbefore+1;
		    len+=digdiff;
		    //printf("digbefore=%d\n",digbefore);
		    //printf("digafter=%d\n",digafter);
		    //printf("digdiff=%d\n",digdiff);
		    strcat(buf,strafterlen);
		    strcat(buf,after);
		  }
		}else{
		  ishtml=0;
		}
		printf("\n------------------------Changed Header---------------------------\n");
		printf("header=%s\n",buf);

		if (my_poll(cl, OUT) <= 0 || write(cl, buf, len) < 1) {
			(void) close(s);
			return -1;
		}
	}
	if (r->type == CONNECT) {
 		char *con_est = "HTTP/1.0 200 Connection established\r\n\r\n";
		int max, sel;
		struct timeval to;
		fd_set fdset;

		to.tv_sec = config.to_con;
		to.tv_usec = 0;

		if (write(cl, con_est, strlen(con_est)) < 1)
			goto c_break;

		if(cl >= s)
			max = cl + 1;
		else
			max = s + 1;

		i = 1;
		sel = 1;
		len = 1;
		while (len > 0 && sel > 0 && i > 0) {
			FD_ZERO(&fdset);
			FD_SET(cl, &fdset);
			FD_SET(s, &fdset);
			sel = select(max, &fdset, (fd_set*) 0, (fd_set*) 0, &to);
			if (FD_ISSET(cl, &fdset)) {
				len = read(cl, buf, sizeof(buf));
				//fprintf(stdout,"HTTPS SENT\n");
				i = write(s, buf, len);
			}
			if (FD_ISSET(s, &fdset)) {
				len = read(s, buf, sizeof(buf));
				//fprintf(stdout,"HTTPS RCVD\n");
				//fprintf(stdout,"%s\n",buf);
				i = write(cl, buf, len);
			}
		}
c_break:
		(void) close(s);
		return 0;
	} else if (r->type != HEAD) {
		while (my_poll(s, IN) > 0 && (len = read(s, buf, sizeof(buf))) > 0) {
                        #ifdef original
                #else
		  
			//fprintf(stdout,"%d\n",dafuq++);
			//char* image;
			//char newurl[]="https://images.pexels.com/photos/18495/pexels-photo.jpg?auto=compress&cs=tinysrgb&h=3x50";
			//if((image=strstr(buf,"<img"))){fprintf(stdout,"image\n");
		  //image=strstr(image+1,"src");strncpy(image+5,newurl,strlen(newurl));
				//fprintf(stdout,"%*s\n",100,image);
				//while((image=strstr(image+1,"<img"))){image=strstr(image+1,"url");image[0]='a';fprintf(stdout,"image\n");//fflush(stdout);}
		  //}
                     //FILE* fp=fopen("file.txt","w");
                     //fprintf(stdout,"%s\n",buf);//fflush(stdout);
			//fclose(fp);
	       #endif
			printf("\n-------------------------Original Body------------------------\n");
			printf("body=%s\n",buf);
			srcnum=0;
			if(ishtml==1){
			  printf("\nBODY WO CHANGE-------------------------------------------------------\n");
			  oafter=buf;
			  while((op=strstr(oafter,"src"))!=NULL){
			    srcnum++;
			    printf("\n1\n");
			    while(*op!='\"'){
			      op++;
			    }
			    printf("\n2\n");
			    op++;
			    *op='\0';
			    originallen=1;
			    while(*op!='\"'){
			      op++;
			      originallen++;
			    }
			    printf("\n3\n");
			    originallen--;
			    difflen=newurllen-originallen;
			    printf("\n3.1\n");
			    printf("\nbuf=%s\n",buf);
			    printf("\nop=%s\n",op);
			    strcpy(string,op);
			    printf("\n4\n");
			    //printf("\nbuf=%s\n",buf);
			    strcat(buf,newurl);
			    printf("\n5\n");			    
			    //printf("\nbuf=%s\n",buf);
			    if(difflen>=0){
			      op+=difflen+1;
			    }
			    q=buf+strlen(buf);
			    strcat(buf,string);
			    printf("\n6\n");			    
			    //printf("\nbuf=%s\n",buf);
			    oafter=q;
			    }
			  
			}
			if(srcnum==0){
			  printf("\nNO SRC!\n");
			}
			  printf("\n-------------------------Changed Body------------------------\n");
			  printf("body=%s\n",buf);

			  if (my_poll(cl, OUT) <= 0 || write(cl, buf, len) < 1) {
			    (void) close(s);
			    return -1;
			  }
			}
			(void) close(s);
			return 0;
	}

	return 0;
}
