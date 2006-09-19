


#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <libsoup/soup-server-auth.h>
#include <wsmand-auth.h>

#include <crypt.h>
#include <time.h>

#include <u/libu.h>


static gboolean 
check_digest_ha1 (
        SoupServerAuthDigest *digest,
        gchar                *ha1)
{
	md5_state_t ctx;
	char hex_a2[33], o[33];
	char *tmp;

	/* compute A2 */
	md5_init (&ctx);
	md5_append (&ctx, 
		    (const md5_byte_t *)digest->request_method, 
		    strlen (digest->request_method));
	md5_append (&ctx, (const md5_byte_t *)":", 1);
	md5_append (&ctx, (const md5_byte_t *)digest->digest_uri, strlen (digest->digest_uri));

	if (digest->integrity) {
		/* FIXME: Actually implement. Ugh. */
		md5_append (&ctx, (const md5_byte_t *)":", 1);
		md5_append (&ctx, (const md5_byte_t *)"00000000000000000000000000000000", 32);
	}

	/* hexify A2 */
	md5_finish (&ctx, (md5_byte_t *)hex_a2);

	/* compute KD */
	md5_init (&ctx);
	md5_append (&ctx, (const md5_byte_t *)ha1, 32);
	md5_append (&ctx, (const md5_byte_t *)":", 1);
	md5_append (&ctx, (const md5_byte_t *)digest->nonce, strlen (digest->nonce));
	md5_append (&ctx, (const md5_byte_t *)":", 1);

	tmp = g_strdup_printf ("%.8x", digest->nonce_count);
	md5_append (&ctx, (const md5_byte_t *)tmp, strlen (tmp));
	g_free (tmp);

	md5_append (&ctx, (const md5_byte_t *)":", 1);
	md5_append (&ctx, (const md5_byte_t *)digest->cnonce, strlen (digest->cnonce));
	md5_append (&ctx, (const md5_byte_t *)":", 1);

	if (digest->integrity)
		tmp = "auth-int";
	else 
		tmp = "auth";

	md5_append (&ctx, (const md5_byte_t *)tmp, strlen (tmp));
	md5_append (&ctx, (const md5_byte_t *)":", 1);

	md5_append (&ctx, (const md5_byte_t *)hex_a2, 32);
	md5_finish (&ctx, (md5_byte_t *)o);

        debug( "expected: %s, actual: %s", digest->digest_response, o);
	return strcmp (o, digest->digest_response) == 0;
}

gboolean authorize_from_file(SoupServerAuth *auth, char *filename)
{
        gboolean             authorized = FALSE;
        char            l[256], u[65], ha1[65], dom[65], passwd[65];
        char *username;
        char *newpw = NULL ;
                    
        username = (char *)soup_server_auth_get_user (auth); 
        debug( "Checking for user: %s", username);
        FILE *fp = g_fopen(filename, "r");
        if (!fp)
            return FALSE;
        switch (auth->type) {
        case SOUP_AUTH_TYPE_BASIC:
            while (fgets(l, sizeof(l), fp) != NULL) {
                if (sscanf(l, "%64[^:]:%64s", u, passwd) != 2)
                    continue;       /* Ignore malformed lines */
                debug( "user: %s,  passwd: %s", u,  passwd);
                if (!strcmp(username, u)) {
                    if (passwd && auth->basic.passwd)
                        newpw = crypt (auth->basic.passwd, passwd);
                        debug( "user: %s,  passwd: %s", u,  newpw);
                    
                        authorized = ( strcmp (newpw, passwd) == 0 );
                    break;
                }
            } 
            break;
        case SOUP_AUTH_TYPE_DIGEST:
            while (fgets(l, sizeof(l), fp) != NULL) {
                if (sscanf(l, "%64[^:]:%64[^:]:%64s", u, dom, ha1) != 3)
                    continue;       /* Ignore malformed lines */
                debug( "user: %s, realm: %s, ha1: %s", u, dom, ha1);
                if (!strcmp(username, u) && !strcmp((char *) (&auth->digest)->realm , dom)) {
                    authorized = check_digest_ha1(&auth->digest, ha1);
                    break;
                }
            } 
            break;
        }

        return (authorized);
} 

