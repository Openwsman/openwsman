/*
 * Copyright (c) 2005, 2006 by KoanLogic s.r.l. - All rights reserved.
 */

static const char rcsid[] =
    "$Id: uri.c,v 1.6 2006/01/09 12:38:38 tat Exp $";
#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif
#include <stdlib.h>
#include <string.h>

#include <u/hash.h>
#include <u/uri.h>
#include <u/carpal.h>
#include <u/misc.h>
#include <u/memory.h>
#include <u/os.h>

/**
 *  \defgroup uri URI
 *  \{
 */

/* split a string separated by 'c' in two substrings */
static int split(const char *s, size_t len, char c, char **left,
		 char **right)
{
	char *buf = 0;
	const char *p;
	char *l = 0, *r = 0;

	buf = u_strndup(s, len);
	nop_err_if(!buf);

	if ((p = strchr(buf, c)) != NULL) {
		l = u_strndup(s, p - buf);
		r = u_strndup(1 + p, len - (p - buf) - 1);
		nop_err_if(!l || !r);
	} else {
		r = NULL;
		nop_err_if((l = u_strndup(buf, len)) == NULL);
	}

	/* return result strings */
	*left = l;
	*right = r;

	U_FREE(buf);

	return 0;
      err:
	U_FREE(buf);
	U_FREE(l);
	U_FREE(r);
	return ~0;
}

static int parse_userinfo(const char *s, size_t len, u_uri_t * uri)
{
	return split(s, len, ':', &uri->user, &uri->pwd);
}

static int parse_hostinfo(const char *s, size_t len, u_uri_t * uri)
{
	char *port = 0;

	if (split(s, len, ':', &uri->host, &port))
		return ~0;

	if (port) {
		uri->port = atoi(port);
		U_FREE(port);
	}
	return 0;
}

static int parse_middle(const char *s, size_t len, u_uri_t * uri)
{
	const char *p;

	if ((p = strchr(s, '@')) == NULL)
		return parse_hostinfo(s, len, uri);
	else
		return parse_userinfo(s, p - s,
				      uri) + parse_hostinfo(1 + p,
							    s + len - p -
							    1, uri);
}

/** \brief dispose memory allocated to URI \a uri */
void u_uri_free(u_uri_t * uri)
{
	if (uri == NULL)
		return;

	U_FREE(uri->scheme);
	U_FREE(uri->user);
	U_FREE(uri->pwd);
	U_FREE(uri->host);
	U_FREE(uri->path);
	U_FREE(uri->query);
	U_FREE(uri);
}

/** \brief parse the URI string \a s and create an \c u_uri_t at \a *pu */
int u_uri_parse(const char *s, u_uri_t ** pu)
{
	const char *p, *p0;
	const char *end, *question;
	char *uri_string=NULL;
	int i;
	u_uri_t *uri;

	dbg_return_if((uri =
		       (u_uri_t *) u_zalloc(sizeof(u_uri_t))) == NULL, ~0);

	dbg_err_if((p = strchr(s, ':')) == NULL);	/* err if malformed */

	/* save schema string */
	dbg_err_if((uri->scheme = u_strndup(s, p - s)) == NULL);

	p++;			/* skip ':' */

	/* skip "//" */
	for (i = 0; i < 2; ++i, ++p)
		dbg_err_if(!p || *p == 0 || *p != '/');	/* err if malformed */

	/* save p */
	p0 = p;

	/* find the first path char ('/') or the end of the string */
	while (*p && *p != '/')
		++p;

	/* parse userinfo and hostinfo */
	dbg_err_if(p - p0 && parse_middle(p0, p - p0, uri));

	/* save path */
	dbg_err_if(*p && (uri->path = u_strdup(p)) == NULL);

	*pu = uri;

	uri_string = u_strdup(s);
	/*
	   end = strchr (uri_string, '#');
	   if (!end)
	 */
	end = uri_string + strlen(uri_string);
	question = memchr(uri_string, '?', end - uri_string);

	if (question) {
		if (question[1]) {
			dbg_err_if((uri->query =
				    u_strndup(question + 1,
					      end - (question + 1))) ==
				   NULL);
		}
	} else {
		uri->query = NULL;
	}
	u_free(uri_string);
	return 0;
      err:
	u_free(uri_string);
	u_uri_free(uri);
	return ~0;
}



// added for openwsman

static int u_string_unify(char *s)
{
	size_t len = strlen(s);
	size_t i, j;
	char n;

	for (i = 0; i < len; i++) {
		if (s[i] != '%') {
			continue;
		}
		if (len - i < 3) {
			return 1;
		}
		if (!isxdigit(s[i + 1]) || !isxdigit(s[i + 2])) {
			return 1;
		}
		n = s[i + 3];
		s[i + 3] = '\0';
		s[i] = (char) strtol(s + i + 1, NULL, 16);
		s[i + 1] = n;
		for (j = i + 4; j <= len; j++) {
			s[j - 2] = s[j];
		}
		len -= 2;
	}

	return 0;
}


hash_t *u_parse_query(const char *query)
{
	char *pp, *tok, *src, *q = NULL;
	char *key=NULL, *val=NULL;
	hash_t *h = NULL;

	dbg_err_if(query == NULL);
	q = u_strdup(query);
	h = hash_create3(HASHCOUNT_T_MAX, 0, 0);

	/* foreach name=value pair... */
	for (src = q; (tok = strtok_r(src, "&,", &pp)) != NULL; src = NULL) {
		/* dup the string so we can modify it */
		key = u_strdup(tok);
		dbg_err_if(key == NULL);

		val = strchr(key, '=');
		dbg_err_if(val == NULL);

		/* zero-term the name part and set the value pointer */
		*val++ = 0;
		val = u_strdup(val);

		u_trim(key);
		u_trim(val);
		u_trim_quotes(val);
		if (u_string_unify(key) || u_string_unify(val)) {
			u_free(key);
			u_free(val);
			dbg("Could not unify query: %s", tok);
			continue;
		}
		if (!hash_lookup(h, key)) {
			if (!hash_alloc_insert(h, key, val)) {
				u_free(key);
				u_free(val);
				warn("hash_alloc_insert failed");
			}
		} else {
			u_free(key);
			u_free(val);
			warn("duplicate not added to hash");
		}
	}
	u_free(q);
	return h;
err:
	u_free(q);
	u_free(key);
	return NULL;
}




/**
 *      \}
 */
