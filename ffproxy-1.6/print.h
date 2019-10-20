#ifdef USE_DEBUG
#define DEBUG(args)	(void) printf args , (void) printf("\n");;
#else
#define DEBUG(args)	;
#endif

void            setup_log_master(void);
void            setup_log_slave(void);
void            fatal(const char *,...);
void            fatal_n(const char *,...);
void            warn(const char *,...);
void            info(const char *,...);
