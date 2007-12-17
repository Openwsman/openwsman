#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif


#include <u/libu.h>


//int facility = LOG_DAEMON;

struct {
        char *uri_string, *result;
} abs_tests[] = {
        { "http://aa.xx.com/test?a=b&c=d", "foo:" },
        { NULL, NULL }
};


int
main(int argc, char *argv[])
{

    int i;
    const char *end, *question;
    char *u_string;

    // parse url
    //
    u_uri_t *uri = u_malloc(sizeof(u_uri_t));

    for (i = 0; abs_tests[i].uri_string != NULL ; i++) {
        (void) console("%s", abs_tests[i].uri_string );
        u_uri_parse(abs_tests[i].uri_string, &uri);
        (void) console("%s", uri->host);
        (void) console("%s", uri->path);
       
        u_string  = u_strdup( abs_tests[i].uri_string );
        end = strchr (u_string, '#');
        if (!end) 
            end = u_string + strlen(u_string);
        question = memchr (u_string, '?', end - u_string);

        char *query = NULL;
        if (question) {
            (void) console("%s", question);
            (void) console("%s", end);
            if (question[1]) {
                query = u_strndup (question + 1, end - (question + 1));
            }
        }
        (void) console("%s", query);

        hash_t *h = u_parse_query(query);
        hnode_t *hn;
        hscan_t hs;     
        hash_scan_begin(&hs, h);
        while ((hn = hash_scan_next(&hs)))
            printf("%s\t%s\n", (char*) hnode_getkey(hn),
                    (char*) hnode_get(hn));

    }
    
    return  0;
}
