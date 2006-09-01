#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <libsoup/soup-server-auth.h>
#include <wsman-md5-utils.h>
#include <wsmand-auth.h>
#include <wsman-debug.h>

#include <crypt.h>
#include <time.h>


static gboolean 
check_digest_ha1 (SoupServerAuthDigest *digest,
	     gchar                *ha1)
{
	WsmanMD5Context ctx;
	char hex_a2[33], o[33];
	char *tmp;

	/* compute A2 */
	wsman_md5_init (&ctx);
	wsman_md5_update (&ctx, 
		    digest->request_method, 
		    strlen (digest->request_method));
	wsman_md5_update (&ctx, ":", 1);
	wsman_md5_update (&ctx, digest->digest_uri, strlen (digest->digest_uri));

	if (digest->integrity) {
		/* FIXME: Actually implement. Ugh. */
		wsman_md5_update (&ctx, ":", 1);
		wsman_md5_update (&ctx, "00000000000000000000000000000000", 32);
	}

	/* hexify A2 */
	wsman_md5_final_hex (&ctx, hex_a2);

	/* compute KD */
	wsman_md5_init (&ctx);
	wsman_md5_update (&ctx, ha1, 32);
	wsman_md5_update (&ctx, ":", 1);
	wsman_md5_update (&ctx, digest->nonce, strlen (digest->nonce));
	wsman_md5_update (&ctx, ":", 1);

	tmp = g_strdup_printf ("%.8x", digest->nonce_count);
	wsman_md5_update (&ctx, tmp, strlen (tmp));
	g_free (tmp);

	wsman_md5_update (&ctx, ":", 1);
	wsman_md5_update (&ctx, digest->cnonce, strlen (digest->cnonce));
	wsman_md5_update (&ctx, ":", 1);

	if (digest->integrity)
		tmp = "auth-int";
	else 
		tmp = "auth";

	wsman_md5_update (&ctx, tmp, strlen (tmp));
	wsman_md5_update (&ctx, ":", 1);

	wsman_md5_update (&ctx, hex_a2, 32);
	wsman_md5_final_hex (&ctx, o);

        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "expected: %s, actual: %s", digest->digest_response, o);
	return strcmp (o, digest->digest_response) == 0;
}

gboolean authorize_from_file(SoupServerAuth *auth, char *filename)
{
        gboolean             authorized = FALSE;
        char            l[256], u[65], ha1[65], dom[65], passwd[65];
        char *username;
        char *newpw = NULL ;
                    
        username = (char *)soup_server_auth_get_user (auth); 
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Checking for user: %s", username);
        FILE *fp = g_fopen(filename, "r");
        if (!fp)
            return FALSE;
        switch (auth->type) {
        case SOUP_AUTH_TYPE_BASIC:
            while (fgets(l, sizeof(l), fp) != NULL) {
                if (sscanf(l, "%64[^:]:%64s", u, passwd) != 2)
                    continue;       /* Ignore malformed lines */
                wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "user: %s,  passwd: %s", u,  passwd);
                if (!strcmp(username, u)) {
                    if (passwd && auth->basic.passwd)
                        newpw = crypt (auth->basic.passwd, passwd);
                        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "user: %s,  passwd: %s", u,  newpw);
                    
                        authorized = ( strcmp (newpw, passwd) == 0 );
                    break;
                }
            } 
            break;
        case SOUP_AUTH_TYPE_DIGEST:
            while (fgets(l, sizeof(l), fp) != NULL) {
                if (sscanf(l, "%64[^:]:%64[^:]:%64s", u, dom, ha1) != 3)
                    continue;       /* Ignore malformed lines */
                wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "user: %s, realm: %s, ha1: %s", u, dom, ha1);
                if (!strcmp(username, u) && !strcmp((char *) (&auth->digest)->realm , dom)) {
                    authorized = check_digest_ha1(&auth->digest, ha1);
                    break;
                }
            } 
            break;
        }

        return (authorized);
} 

