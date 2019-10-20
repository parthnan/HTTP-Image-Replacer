#include <regex.h>

extern regex_t *a_ip[];      //these a_ ones are used in access.c
extern regex_t *a_host[];
extern char    *a_dyndns[];
extern regex_t *f_host[];
extern regex_t *f_url[];
extern regex_t *f_hdr_drop[];
extern regex_t *f_hdr_match[];
extern char    *f_hdr_entry[];
extern char    *f_hdr_add[];
extern regex_t *f_rhdr_drop[];
extern regex_t *f_rhdr_match[];
extern char    *f_rhdr_entry[];
extern char    *e_inv;
extern char    *e_res;
extern char    *e_con;
extern char    *e_post;
extern char    *e_fil;
extern char    *e_nic;
