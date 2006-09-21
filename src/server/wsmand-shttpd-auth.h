
#ifndef WSMAND_SHTTPD_AUTH_H_
#define WSMAND_SHTTPD_AUTH_H_

typedef struct {
        const char *request_method;
        int        integrity;
        const char *username;
        const char *realm;
        const char *digest_uri;
        int        nonce_count;
        const char *nonce;
        const char *cnonce;
        const char *digest_response;
} WSmanAuthDigest;


int ws_authorize_digest(char *filename, WSmanAuthDigest *digest);
int ws_authorize_basic(char *filename, char *username, const char *password);

#endif
