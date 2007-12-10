
#ifndef ADAPTER_HEADER_DEFINED
#define	ADAPTER_HEADER_DEFINED

void
shttpd_get_credentials(struct shttpd_arg *arg, char **user, char **pwd);
int open_listening_port(int port);
char *shttpd_reason_phrase(int code);


#endif
