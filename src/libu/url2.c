
#include <u/libu.h>
#include <u/url2.h>

#define MAXTOKENS 64

hash_t *parse_query(char *query)
{
    char *pp, *tok, *src, *q = NULL;
    char *key, *val;

    q = u_strdup(query);
    dbg_err_if(query == NULL);
    hash_t *h = hash_create(HASHCOUNT_T_MAX, 0, 0);

    /* foreach name=value pair... */
    for(src = query; (tok = strtok_r(src, "&", &pp)) != NULL; src = NULL)
    {
        /* dup the string so we can modify it */
        key = u_strdup(tok);
        dbg_err_if(key == NULL);

        val = strchr(key, '=');
        dbg_err_if(val == NULL);

        /* zero-term the name part and set the value pointer */
        *val++ = 0; 
        if (!hash_alloc_insert(h, key, val)) {
            debug("hash_alloc_insert failed");
        }
    }

    u_free(query);

    return h;
err:
    u_free(query);
    return h;
}


