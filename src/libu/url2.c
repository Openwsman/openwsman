


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


#if 0
hash_t* parse_query ( char *query )
{
    char *q = NULL;
    hash_t *h = hash_create(HASHCOUNT_T_MAX, 0, 0);
    char **vec, **l;
    char *key, *val;
    int i;
    if (query && query[0])
    {
        //dbg_return_if ((q = u_strdup(query)) == NULL, 0);
        u_tokenize (query, "&\0\n", vec,  MAXTOKENS);
        for (l = vec; *l; l++) {
            key = u_strdup(*l);
            val = strchr (key, '=');
            if(val) {
                *val = 0;
                val++;
            }
            if (!hash_alloc_insert(h, key, val)) {
                debug("hash_alloc_insert failed");
            }
        }
    }
        u_strfreev(vec);
    //u_free(q);
    return h;
}
#endif

