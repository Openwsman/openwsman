/*
 * Copyright (c) 2005, 2006 by KoanLogic s.r.l. - All rights reserved.
 */

#ifndef _U_URI_H_
#define _U_URI_H_


#ifdef __cplusplus
extern "C" {
#endif

struct u_uri_s
{
    char *scheme;
    char *user;
    char *pwd;
    char *host;
    short port;
    char *path;
    char *query;
};

typedef struct u_uri_s u_uri_t;

int u_uri_parse (const char *s, u_uri_t **pu);
void u_uri_free (u_uri_t *uri);
hash_t *u_parse_query(const char *query);

#ifdef __cplusplus
}
#endif

#endif /* !_U_URI_H_ */ 
