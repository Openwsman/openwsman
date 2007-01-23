#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include <u/libu.h>
#include <u/url2.h>

#define MAXTOKENS 64


static int unify(char *s)
{
    int len = strlen(s);
    int i, j;
    char n;

    for (i = 0; i < len; i++) {
        if (s[i] !='%') {
            continue;
        }
        if (len - i < 3) {
            return 1;
        }
        if (!isxdigit(s[i+1]) || !isxdigit(s[i+2])) {
            return 1;
        }
        n = s[i+3];
        s[i+3] = '\0';
        s[i] = (char)strtol(s + i + 1, NULL, 16);
        s[i + 1] = n;
        for (j = i + 4; j <= len; j++) {
            s[j - 2] = s[j];
        }
        len -= 2;
    }

    return 0;
}


hash_t *parse_query(const char *query)
{
    char *pp, *tok, *src, *q = NULL;
    char *key, *val;
    hash_t *h = NULL;

    dbg_err_if(query == NULL);
    q = u_strdup(query);
    h = hash_create(HASHCOUNT_T_MAX, 0, 0);

    /* foreach name=value pair... */
    for(src = q; (tok = strtok_r(src, "&,", &pp)) != NULL; src = NULL)
    {
        /* dup the string so we can modify it */
        key = u_strdup(tok);
        dbg_err_if(key == NULL);

        val = strchr(key, '=');
        dbg_err_if(val == NULL);

        /* zero-term the name part and set the value pointer */
        *val++ = 0; 
        if (unify(key) || unify(val)) {
            u_free(key);
            debug("Could not unify query: %s", tok);
            continue;
        }
        if (!hash_lookup(h,key)) {
            if ( !hash_alloc_insert(h, key, val)) {
                error("hash_alloc_insert failed");
            }
        } else {
            error("duplicate not added to hash");
        }
    }
    /*
    hscan_t hs;
    hnode_t *hn;
    hash_scan_begin(&hs, h);
    while ((hn = hash_scan_next(&hs))) {    	
        printf("xxx: %s\n",  (char*) hnode_getkey(hn));   
    } 
    */   
    u_free(q);
    return h;
err:
	u_free(q);
    return NULL;
}


