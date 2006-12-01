#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include <u/libu.h>

int facility = LOG_DAEMON;

typedef struct _entry_t {
    lnode_t n;
    char *name;
} entry_t;


int
main(int argc, char *argv[])
{

    list_t *l = list_create(LISTCOUNT_T_MAX);
    entry_t *e = (entry_t *)u_malloc(sizeof(entry_t));
    char *x = "foo";
    e->name = x;
    e->n.list_data = (void *)"bar";

    list_append(l, &e->n);


    entry_t *y = (entry_t *)list_first(l);
    while (y != NULL) {
        char *name = (char *)y->n.list_data;
        e = (entry_t *)y;
        printf("%s\n", name);
        printf("%s\n", y->name);
        y = (entry_t *)list_next(l, &(y->n) );
    }
    
    
    return  0;
}
