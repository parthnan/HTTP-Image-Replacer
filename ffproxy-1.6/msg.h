#ifndef HAD_REQ_H
#include "req.h"
#endif

struct msg {
	char           *c;
	int             len;
};

enum {
	E_INV = 10, E_RES, E_CON, E_POST, E_FIL
};

void            err_msg(int, struct req *, int);
