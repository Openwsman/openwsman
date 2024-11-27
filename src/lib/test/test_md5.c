#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif


#include <u/libu.h>

int facility = LOG_DAEMON;

int main(int argc, char **argv)
{
    static const char *const test[7] = {
        "", /*d41d8cd98f00b204e9800998ecf8427e*/
        "a", /*0cc175b9c0f1b6a831c399e269772661*/
        "abc", /*900150983cd24fb0d6963f7d28e17f72*/
        "message digest", /*f96b697d7cb7938d525a2f31aaf161d0*/
        "abcdefghijklmnopqrstuvwxyz", /*c3fcd3d76192e4007dfb496cca67e13b*/
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
        /*d174ab98d277d9f5a5611c2c9f419d9f*/
        "12345678901234567890123456789012345678901234567890123456789012345678901234567890" /*57edf4a22be3c955ac49da2e2107b67a*/
    };
    int i;

    for (i = 0; i < 7; ++i) {
        md5_state_t state;
        md5_byte_t digest[16];
        int di;

        md5_init(&state);
        md5_append(&state, (const md5_byte_t *)test[i], strlen(test[i]));
        md5_finish(&state, digest);
        printf("MD5 (\"%s\") = ", test[i]);
        for (di = 0; di < 16; ++di)
            printf("%02x", digest[di]);
        printf("\n");
    }
    md5_state_t ctx;
    /* computer A1 */
    md5_byte_t HA1[16];
    md5_init (&ctx);
    md5_append (&ctx, (const md5_byte_t *)"Mufasa", 6 );
    md5_append (&ctx, (const md5_byte_t *)":", 1);
    md5_append (&ctx, (const md5_byte_t *)"testrealm@host.com", 18);
    md5_append (&ctx, (const md5_byte_t *)":", 1);
    md5_append (&ctx, (const md5_byte_t *)"Circle Of Life", 14);
    md5_finish(&ctx, HA1);
    printf("HA2: %s\n",  md52char(HA1));


    /* compute A2 */
    md5_byte_t HA2[16];
    md5_init (&ctx);
    md5_append (&ctx, (const md5_byte_t *)"GET", 3 );
    md5_append (&ctx, (const md5_byte_t *)":", 1);
    md5_append (&ctx, (const md5_byte_t *)"/dir/index.html", 15);
    md5_finish(&ctx, HA2);

    printf("HA2: %s\n",  md52char(HA2));

    /* compute Response */
    md5_byte_t Response[16];
    md5_init (&ctx);
    md5_append (&ctx, (const md5_byte_t *)md52char(HA1), strlen(md52char(HA1)) );
    md5_append (&ctx, (const md5_byte_t *)":", 1);
    md5_append (&ctx, (const md5_byte_t *)"dcd98b7102dd2f0e8b11d0f600bfb0c093", 34);
    md5_append (&ctx, (const md5_byte_t *)":", 1);
    md5_append (&ctx, (const md5_byte_t *)"00000001", 8);
    md5_append (&ctx, (const md5_byte_t *)":", 1);
    md5_append (&ctx, (const md5_byte_t *)"0a4f113b", 8 );
    md5_append (&ctx, (const md5_byte_t *)":", 1);
    md5_append (&ctx, (const md5_byte_t *)"auth", 4);
    md5_append (&ctx, (const md5_byte_t *)":", 1);
    md5_append (&ctx, (const md5_byte_t *)md52char(HA2), strlen(md52char(HA2)) );
    md5_finish(&ctx, Response);
    printf("Response: %s\n",  md52char(Response));

    return 0;
}
