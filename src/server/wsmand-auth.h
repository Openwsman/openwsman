
#ifndef WSMAND_AUTH_H_
#define WSMAND_AUTH_H_

typedef struct {
        const char *request_method;
        const char *qop;
        const char *username;
        const char *realm;
        const char *digest_uri;
        const char *nonce;
        const char *cnonce;
        const char *digest_response;
        char nonce_count[12];
} WSmanAuthDigest;


int ws_authorize_digest(char *filename, WSmanAuthDigest *digest);
#ifdef LIBSOUP_LISTENER
int ws_authorize_basic(char *username, const char *password);
#endif

#endif
