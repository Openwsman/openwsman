

#include "shttpd_defs.h"
#include "adapter.h"
#ifdef SHTTPD_GSS
void getGssName(struct conn *c, char **user);
#endif
void
shttpd_get_credentials(struct shttpd_arg *arg,
                char **user, char **pwd)
{
	char *p, *pp;
	struct conn *c = (struct conn *)arg->priv;
	struct vec 	*auth_vec = &c->ch.auth.v_vec;
	if (auth_vec->len > 10 && !strncasecmp(auth_vec->ptr, "Basic ", 6)) {
		char buf[4096];
		int l;

		p = (char *) auth_vec->ptr + 5;
		while ((*p == ' ') || (*p == '\t')) {
			p++;
		}
		pp = p;
		while ((*p != ' ') && (*p != '\t') && (*p != '\r')
				&& (*p != '\n') && (*p != 0)) {
			p++;
		}

		if (pp == p) {
			return ;
		}
		*p = 0;

		l = ws_base64_decode(pp, p - pp, buf, 4095);
		if (l <= 0) {
			return ;
		}

		buf[l] = 0;
		p = buf;
		pp = p;
		p = strchr(p, ':');
		if (p == NULL) {
			return ;
		}
		*p++ = 0;

		*user = u_strdup(pp);
		*pwd = u_strdup(p);
	}
#ifdef SHHTPD_GSS
    else if(!my_strncasecmp(auth_vec->ptr, "Kerberos ", 9))
    {
        getGssName(c, user);
        *pwd = 0;
    }
#endif
}

char *shttpd_reason_phrase(int code)
{
    struct http_code_map {
    	int code;
        char *reason_phrase;
    };
    struct http_code_map maps[] = {
        {200, "OK"},
        {400, "Bad request"},
        {401, "Unauthorized"},
        {403, "Forbidden"},
        {404, "Not found"},
        {500, "Internal Error"},
        {501, "Not implemented"},
        {415, "Unsupported Media Type"},
        {0, NULL}
    };
    int i = 0;
    while(maps[i].code) {
        if(maps[i].code == code)
        	return maps[i].reason_phrase;
        i++;
    }
    return NULL;
}
