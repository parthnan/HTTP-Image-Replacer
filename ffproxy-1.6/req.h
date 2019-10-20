#define HAD_REQ_H

struct clinfo {
	char            name[128];
	char            ip[128];
};

struct req {
	char            url[2048];
	char		urlpath[2048];
	char            host[128];
	unsigned int    port;

	int             type;
	int             relative;
	int		kalive;
	int             vmajor;
	int             vminor;

	long            clen;
	char            tstamp[32];
	char            ctype[32];

	char           *header[32];

	char            fname[256];
	char            lname[257];

	int             loop;

	struct clinfo  *cl;
};

enum {
	GET = 0, POST, HEAD, CONNECT, UNKNOWN
};
