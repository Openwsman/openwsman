

#include "defs.h"
#include "adapter.h"


void
shttpd_get_credentials(struct shttpd_arg *arg,
                char **user, char **pwd)
{
        struct conn *c = (struct conn *)arg->priv;
        *user = c->username;
        *pwd =  c->password;
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
