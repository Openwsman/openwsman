#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "glib.h"
#include "u/libu.h"
#include <wsmand-shttpd-auth.h>

#include <crypt.h>
#include <time.h>


static int 
check_digest_ha1 (WSmanAuthDigest *digest,
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


int
ws_authorize_digest(char *filename, WSmanAuthDigest *digest)
{
        int             authorized = 0;
        char            l[256], u[65], ha1[65], dom[65];

        debug( "Checking for user: %s in digest", digest->username);

        FILE *fp = fopen(filename, "r");
        if (!fp) {
            debug( "Couldn't open digest passwd file %s",
                        filename);
            return 0;
        }       
        while (fgets(l, sizeof(l), fp) != NULL) {
                if (sscanf(l, "%64[^:]:%64[^:]:%64s", u, dom, ha1) != 3) {
                        continue;       /* Ignore malformed lines */
                }
                debug( "user: %s, realm: %s, ha1: %s", u, dom, ha1);
                if (!strcmp(digest->username, u) && !strcmp(digest->realm , dom)) {
                    authorized = check_digest_ha1(digest, ha1);
                    break;
                }
        }

        return (authorized);
}         
        
int
ws_authorize_basic(char *filename, char *username, const char *password)
{        
        int             authorized = 0;
        char            l[256], u[65], passwd[65];
        char *newpw = NULL ;
         
        debug( "Checking for user: %s", username);
        
        if ((username == NULL) || (password == NULL)) {
                return 0;
        }
        FILE *fp = fopen(filename, "r");
        if (!fp) {
            debug( "Couldn't open basic passwd file %s",
                        filename);            
            return 0;
        }

        while (fgets(l, sizeof(l), fp) != NULL) {
                if (sscanf(l, "%64[^:]:%64s", u, passwd) != 2)
                    continue;       /* Ignore malformed lines */
                debug( "user: %s,  passwd: %s", u,  passwd);
                if (!strcmp(username, u)) {
                        newpw = crypt(password, passwd);
                        debug( "user: %s,  passwd: %s", u,  newpw);
                        authorized = ( strcmp (newpw, passwd) == 0 );
                    break;
                }
       }
       
       return authorized;
}       

