/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef SHTTPD_HEADER_INCLUDED
#define	SHTTPD_HEADER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*
 * This structure is passed to the user callback function
 */
struct shttpd_arg_t {
	void		*priv;		/* Do not touch! SHTTPD private. */
	void		*state;		/* User state */
	int		last;		/* Marks the last call */
	void		*user_data;	/* User-defined data */
	char		*buf;		/* Buffer to fill */
	size_t		buflen;		/* Buffer length */
};


struct shttpd_ctx;

#ifdef OPENWSMAN
/*
 * HTTP digest authentication
 */
#include "u/libu.h"
 

#ifndef IO_MAX
#define	IO_MAX		16384		/* Max request size		*/
#endif /* IO_MAX */
#ifndef USER_MAX
#define	USER_MAX	64		/* Remote user name maxsize	*/
#endif

struct digest {
	char		user[USER_MAX];
	char		uri[IO_MAX];
	char		cnonce[64];
	char		nonce[33];
	char		resp[33];
	char		qop[16];
	char		nc[16];
};

struct basic {
	char		user[USER_MAX];
        char            password[77];
};

typedef int (*shttpd_bauth_callback_t)(char *u, char *passwd);
typedef int (*shttpd_dauth_callback_t)(char *realm, char *method, struct digest *);

 
extern void shttpd_register_bauth_callback(struct shttpd_ctx *,
                            shttpd_bauth_callback_t);
extern void shttpd_register_dauth_callback(struct shttpd_ctx *,
                            shttpd_dauth_callback_t);
extern void      shttpd_get_credentials(struct shttpd_arg_t *,
                        char **user, char **pwd);
#endif

/*
 * User callback function. Called when certain registered URLs have been
 * requested. These are the requirements to the callback function:
 *
 * - it must copy data into supplied buffer and return number of bytes copied
 * - it must not block the execution
 * - it must set 'last' in shttpd_arg_t to 1 if there is no more data.
 */
typedef int (*shttpd_callback_t)(struct shttpd_arg_t *);

/*
 * shttpd_init		Initialize shttpd context. Parameters: configuration
 *			file name (may be NULL), then NULL-terminated
 *			sequence of pairs "option_name", "option_value".
 * shttpd_fini		Dealocate the context
 * shttpd_open_port	Opens non-blocking socket on specified port.
 * shttpd_register_url	Setup the callback function for specified URL.
 * shttpd_protect_url	Associate authorization file with an URL.
 * shttpd_addmimetype	Add mime type
 * shtppd_listen	Add a listening socket to the SHTTPD instance
 * shttpd_poll		Do connections processing
 * shttpd_get_var	Return POST/GET variable value for given variable name.
 * shttpd_version	return string with SHTTPD version
 * shttpd_get_header	return value of the specified HTTP header
 * shttpd_get_env	return string values for the following
 *			pseudo-variables: "REQUEST_METHOD",
 *			"REMOTE_USER" and "REMOTE_ADDR".
 */


extern struct shttpd_ctx *shttpd_init(const char *config_file, ...);
extern void		shttpd_fini(struct shttpd_ctx *);
extern void		shttpd_addmimetype(struct shttpd_ctx *,
				const char *ext, const char *mime);
extern int		shttpd_open_port(int port);
extern void		shttpd_listen(struct shttpd_ctx *ctx, int sock);
extern void		shttpd_register_url(struct shttpd_ctx *ctx,
				const char *url, shttpd_callback_t callback,
				void *user_data);
extern void		shttpd_protect_url(struct shttpd_ctx *ctx,
				const char *url, const char *file);
extern void		shttpd_poll(struct shttpd_ctx *, int milliseconds);

extern const char *	shttpd_version(void);
extern const char *	shttpd_get_var(struct shttpd_arg_t *, const char *var);
extern const char *	shttpd_get_header(struct shttpd_arg_t *, const char *);
extern const char *	shttpd_get_env(struct shttpd_arg_t *, const char *);

extern int		shttpd_get_post_query_len(struct shttpd_arg_t *);
extern int              shttpd_get_post_query(struct shttpd_arg_t *, char *, int);
extern int		shttpd_get_http_version(struct shttpd_arg_t *);
extern const char *     shttpd_get_uri(struct shttpd_arg_t *);




/*
 * The following three functions are for applications that need to
 * load-balance the connections on their own. Many threads may be spawned
 * with one SHTTPD context per thread. Boss thread may only wait for
 * new connections by means of shttpd_accept(). Then it may scan thread
 * pool for the idle thread by means of shttpd_active(), and add new
 * connection to the context by means of shttpd_add().
 */
extern void		shttpd_add(struct shttpd_ctx *, int sock);
extern int		shttpd_accept(int lsn_sock, int milliseconds);
extern int		shttpd_active(struct shttpd_ctx *);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SHTTPD_HEADER_INCLUDED */
