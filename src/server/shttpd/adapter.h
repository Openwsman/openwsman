
#ifndef ADAPTER_HEADER_DEFINED
#define	ADAPTER_HEADER_DEFINED

void
shttpd_get_credentials(struct shttpd_arg *arg, char **user, char **pwd);
char *shttpd_reason_phrase(int code);


#endif
