struct cfg {
	unsigned int    port;
	
	char		ipv4[256];
	char		ipv6[256];

	int             daemon;
	int             childs;
	int             ccount;
	int             backlog;

	unsigned long	uid;
	unsigned long	gid;
	char            chroot[256];
	char            dbdir[256];
	char            file[256];

	char		proxyhost[256];
	unsigned int    proxyport;

	int             syslog;
	int             logrequests;

	int		use_ipv6;
	int		aux_proxy_ipv6;

	int		bind_ipv6;
	int		bind_ipv4;

	int		accel;
	int		accelusrhost;
	char		accelhost[256];
	unsigned int	accelport;

	int		kalive;

	int		unr_con;
	int		to_con;

	int		nowarn;
	int		first;
};

#define MAX_CHILDS	1024
#define MAX_BACKLOG	64
#define MAX_PORTS	65535
#define MAX_UID		65535
#define MAX_GID		MAX_UID
#define MAX_FSIZE	256*1024
