#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "u/libu.h"
#include <wsmand-auth.h>
#include "wsmand-daemon.h"

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#include <time.h>


static int 
check_digest_ha1 (WSmanAuthDigest *digest,
	          char *ha1)
{
	char *resp;
	md5_state_t ctx;
	md5_byte_t hex_a2[16];
        md5_byte_t o[16];


	/* compute A2 */
	md5_init (&ctx);
	md5_append (&ctx, 
		    (const md5_byte_t *)digest->request_method, 
		    strlen (digest->request_method));
	md5_append (&ctx, (const md5_byte_t *)":", 1);
	md5_append (&ctx, (const md5_byte_t *)digest->digest_uri, strlen (digest->digest_uri));

	if (!strncmp(digest->qop, "auth-int", 8)) {
		/* FIXME: Actually implement. Ugh. */
		md5_append (&ctx, (const md5_byte_t *)":", 1);
		md5_append (&ctx, (const md5_byte_t *)"00000000000000000000000000000000", 32);
	}

	/* hexify A2 */
	md5_finish (&ctx, hex_a2);

	/* compute KD */
	md5_init (&ctx);
	md5_append (&ctx, (const md5_byte_t *)ha1, strlen(ha1));
	md5_append (&ctx, (const md5_byte_t *)":", 1);
	md5_append (&ctx, (const md5_byte_t *)digest->nonce, strlen (digest->nonce));
	md5_append (&ctx, (const md5_byte_t *)":", 1);

	md5_append (&ctx, (const md5_byte_t *)digest->nonce_count,
                strlen (digest->nonce_count));

	md5_append (&ctx, (const md5_byte_t *)":", 1);
	md5_append (&ctx, (const md5_byte_t *)digest->cnonce, strlen (digest->cnonce));
	md5_append (&ctx, (const md5_byte_t *)":", 1);


	md5_append (&ctx, (const md5_byte_t *)digest->qop,
                strlen (digest->qop));
	md5_append (&ctx, (const md5_byte_t *)":", 1);

	md5_append (&ctx,  (const md5_byte_t *)md52char(hex_a2), strlen(md52char(hex_a2)));
	md5_finish (&ctx, o);

        resp = md52char(o);
        debug( "expected: %s, actual: %s", digest->digest_response, resp);
	return strcmp (resp, digest->digest_response) == 0;
}


int
ws_authorize_digest(char *filename, WSmanAuthDigest *digest)
{
        int             authorized = 0;
        char            l[256], u[65], ha1[65], dom[65];

        debug( "Checking for user: %s in digest", digest->username);

        FILE *fp = fopen(filename, "r");
		debug( "Checking for user: %s in digest", digest->username);
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
                    debug("check response");
                    authorized = check_digest_ha1(digest, ha1);
                    break;
                }
        }

        fclose(fp);

        return (authorized);
}

#ifdef LIBSOUP_LISTENER

int
ws_authorize_basic(char *username, const char *password)
{
        char *filename = wsmand_options_get_basic_password_file();
        int             authorized = 0;
        char            l[256], u[65], passwd[65];
        char *newpw = NULL ;

        debug( "Checking basic for user: %s; password %s",
                            username, password);
        if (filename == NULL) {
            debug("Could not get password file name");
            return 0;
        }

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

       fclose(fp);

       return authorized;
}

#endif




