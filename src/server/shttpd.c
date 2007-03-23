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

/*
 * Small and portable HTTP server, http://shttpd.sourceforge.net
 *
 * Compilation:
 *    No win32 GUI:		-DNO_GUI
 *    No CGI:			-DNO_CGI
 *    Override max request size	-DIO_MAX=xxxx
 *    Default config file:	-DCONFIG=\"/etc/shttpd.conf\"
 *    Typedef socklen_t		-DNO_SOCKLEN_T
 *    Use embedded:		-DEMBEDDED
 */


#define OPENWSMAN

#ifdef OPENWSMAN
#ifndef EMBEDDED
#define EMBEDDED
#endif
#include "u/libu.h"
#include "shttpd.h"
#ifdef HAVE_CONFIG_H
#include "wsman_config.h"
#endif
#endif


#define	SHTTPD_VERSION		"1.35"		/* Version			*/
#ifndef CONFIG
#define	CONFIG		"/usr/local/etc/shttpd.conf"	/* Configuration file		*/
#endif /* CONFIG */
#define	HTPASSWD	".htpasswd"	/* Passwords file name		*/
#define	EXPIRE_TIME	3600		/* Expiration time, seconds	*/
#define KEEP_ALIVE_TIME 2      /* keep connection, seconds */
#ifndef IO_MAX
#define	IO_MAX		16384		/* Max request size		*/
#endif /* IO_MAX */
#ifndef USER_MAX
#define	USER_MAX	64		/* Remote user name maxsize	*/
#endif /* USER_MAX */
#define	AUTH_MAX	1024		/* Authorization line		*/
#define	NVAR_MAX	128		/* Maximum POST variables	*/
#define	PORT		"80"		/* Default listening port	*/
#define	INDEX_FILES	"index.html,index.php,index.cgi" /* Index files	*/
#define	CGI_EXT		".cgi"		/* Default CGI extention	*/
#define	REALM		"mydomain.com"	/* Default auth realm		*/
#define	ENV_MAX		4096		/* Size of environment block	*/

#define	NELEMS(ar)	(sizeof(ar) / sizeof(ar[0]))


#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#endif


#include <pwd.h>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
//#define	SSL_LIB				"/usr/lib/libssl.so"
#define SSL_LIB             "libssl.so"
#define	DIRSEP				'/'
#define	O_BINARY			0
#define	closesocket(a)			close(a)
#define	ERRNO				errno
#define	NO_GUI

#define	InitializeCriticalSection(x)	/* FIXME UNIX version is not MT safe */
#define	EnterCriticalSection(x)
#define	LeaveCriticalSection(x)

#include <sys/types.h>		/* Common #includes (ANSI and SSL) */
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <fcntl.h>

#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif /* WITH_DMALLOC */



/*
 * Darwin prior to 7.0 and Win32 do not have socklen_t
 */
#ifdef NO_SOCKLEN_T
typedef int socklen_t;
#endif


/*
 * Snatched from OpenSSL includes. I put the prototypes here to be independent
 * from the OpenSSL source installation. Having this, shttpd + SSL can be
 * built on any system with binary SSL libraries installed.
 */

#ifdef HAVE_SSL
#include <openssl/ssl.h>
/*  bug #67
typedef struct ssl_st SSL;
typedef struct ssl_method_st SSL_METHOD;
typedef struct ssl_ctx_st SSL_CTX;
*/
#endif

#if 0
#define	SSL_ERROR_WANT_READ	2
#define	SSL_ERROR_WANT_WRITE	3
#define SSL_FILETYPE_PEM	1
#endif
/*
 * Unified socket address
 */
struct usa {
	socklen_t len;
	union {
		struct sockaddr	sa;
		struct sockaddr_in sin;
	} u;
};

/*
 * Mime type entry
 */
struct mimetype {
	struct mimetype	*next;
	char		*ext;		/* File extention	*/
	char		*mime;		/* Mime type		*/
	size_t		extlen;		/* Extention length	*/
};

/*
 * Known HTTP methods
 */
enum {METHOD_GET, METHOD_POST, METHOD_PUT, METHOD_DELETE, METHOD_HEAD};

/*
 * I/O buffer
 */
struct io {
	char	*buf;		/* Buffer		*/
	size_t  bufsize;
	int	done;			/* IO finished		*/
	size_t	head;			/* Bytes read		*/
	size_t	tail;			/* Bytes written	*/
};
#define	IO_SPACELEN(io)		((io)->bufsize - (io)->head - 1)
#define	IO_DATALEN(io)		((io)->head - (io)->tail)

/*
 * Connection descriptor
 */
#define	PROTO_SIZE	16
struct shttpd_ctx;
typedef void (*shttpd_watch_t)(struct shttpd_ctx *, void *);
struct conn {
	struct conn	*next;		/* Connections chain		*/
	struct shttpd_ctx *ctx;		/* Context this conn belongs to */
	struct usa	sa;		/* Remote socket address	*/
	time_t		birth;		/* Creation time		*/
	time_t		expire;		/* Expiration time		*/
	time_t		ims;		/* If-Modified-Since:		*/
	int		sock;		/* Remote socket		*/
#ifdef HAVE_SSL
	SSL		*ssl;		/* SSL descriptor		*/
#endif
	int		reqlen;		/* Request length		*/
	int		status;
	int		http_method;	/* HTTP method			*/
	void		*state;		/* Embedded. Callback state.	*/
	unsigned long	cclength;	/* Client Content-Length	*/
	unsigned long	sclength;	/* Server Content-Length	*/
	unsigned long	shlength;	/* Server headers length	*/
	unsigned long	nsent;		/* Bytes sent to client		*/
	shttpd_watch_t	watch;		/* IO readiness callback	*/
	void		*watch_data;	/* Callback data		*/

	struct io	local;		/* Local IO buffer		*/
	struct io	remote;		/* Remote IO buffer		*/
	void (*io)(struct conn *);	/* Local IO function		*/

	char		method[16];	/* Used method			*/
	char		uri[IO_MAX];	/* Url-decoded URI		*/
	char		saved[IO_MAX];	/* Saved request		*/
	char		proto[PROTO_SIZE];	/* HTTP protocol	*/

	char		*user;		/* Remote user name		*/
	char		*auth;		/* Authorization		*/
	char		*useragent;	/* User-Agent:			*/
	char		*path;		/* Path for get_dir		*/
	char		*referer;	/* Referer:			*/
	char		*cookie;	/* Cookie:			*/
	char		*ctype;		/* Content-Type:		*/
	char		*location;	/* Location:			*/
	char		*query;		/* QUERY_STRING			*/
	char		*range;		/* Range:			*/
	char		*path_info;	/* PATH_INFO thing		*/
	char        *usr;       /* save username if basic authentication */
	char        *pwd;       /* save password if basic authentication */

	unsigned long	nposted;	/* Emb. POST bytes buffered	*/
	void		*userurl;	/* For embedded data		*/
	char		*vars[NVAR_MAX];	/* Variables		*/

	int		fd;		/* Local file descriptor	*/
	struct stat	st;		/* Stats of requested file	*/
	DIR		*dirp;		/* Opened directory		*/
	unsigned int	flags;		/* Flags			*/
#define	FLAG_FINISHED           0x0001	/* Connection to be closed	*/
#define	FLAG_PARSED             0x0002	/* Request has been parsed	*/
#define	FLAG_CGIPARSED          0x0004	/* CGI output has been parsed	*/
#define	FLAG_SSLACCEPTED        0x0008	/* SSL_accept() succeeded	*/
#define	FLAG_ALWAYS_READY       0x0010	/* Local channel always ready for IO */
	//#define	FLAG_USER_WATCH         0x0020	/* User watch			*/
#define	FLAG_SOCK_READABLE      0x0040
#define	FLAG_SOCK_WRITABLE      0x0080
#define FLAG_FD_READABLE        0x0100
#define	FLAG_FD_WRITABLE        0x0200
#define	FLAG_CGI                0x0400
#define	FLAG_KEEP_CONNECTION	0x0800
#define FLAG_AUTHORIZED         0x1000  /* Already authorized */
#define FLAG_HAVE_TO_WRITE      0x2000  /* connection have something to write */
};

#define	FLAG_IO_READY	(FLAG_SOCK_WRITABLE | FLAG_SOCK_READABLE | \
		FLAG_FD_READABLE | FLAG_FD_WRITABLE)

enum err_level	{ERR_DEBUG, ERR_INFO, ERR_FATAL};

#define elog(level, ...) \
	do { \
		debug( __VA_ARGS__ ); \
		if (level == ERR_FATAL) exit(EXIT_FAILURE); \
	} while (0)

enum hdr_type	{HDR_DATE, HDR_INT, HDR_STRING};

/*
 * Known HTTP headers
 */
#define	OFFSET(x)	offsetof(struct conn, x)
struct header {
	size_t		len;		/* Header name length		*/
	enum hdr_type	type;		/* Header type			*/
	size_t		offset;		/* Where to store a header value*/
	const char	*name;		/* Header name			*/
} headers[] = {
	{12, HDR_STRING, OFFSET(useragent),	"User-Agent: "		},
	{14, HDR_STRING, OFFSET(ctype),		"Content-Type: "	},
	{16, HDR_INT,	 OFFSET(cclength),	"Content-Length: "	},
	{19, HDR_DATE,	 OFFSET(ims),		"If-Modified-Since: "	},
	{15, HDR_STRING, OFFSET(auth),		"Authorization: "	},
	{9,  HDR_STRING, OFFSET(referer),	"Referer: "		},
	{8,  HDR_STRING, OFFSET(cookie),	"Cookie: "		},
	{10, HDR_STRING, OFFSET(location),	"Location: "		},
	{8,  HDR_INT,	 OFFSET(status),	"Status: "		},
	{7,  HDR_STRING, OFFSET(range),		"Range: "		},
	{0,  HDR_INT,	 0,			NULL			}
};

union variant {
	char		*value_str;
	unsigned long	value_int;
	time_t		value_time;
	void		(*value_func)(void);
	void		*value_void;
};


/*
 * Mount points (Alias)
 */
struct mountpoint {
	struct mountpoint	*next;
	char			*path;
	char			*mountpoint;
};

/*
 * Environment variables
 */
struct envvar {
	struct envvar		*next;
	char			*name;
	char			*value;
};

/*
 * User-registered URI
 */
struct userurl {
	struct userurl		*next;
	char		*url;
	shttpd_callback_t	func;
	int authnotneeded;
	void			*data;
};

/*
 * User defined authorization file
 */
struct userauth {
	struct userauth		*next;
	const char		*url;
	const char		*filename;
};

/*
 * SHTTPD context
 */
struct shttpd_ctx {
	time_t		start_time;		/* Start time */
	int		nactive;		/* # of connections now */
	unsigned int	nrequests;		/* Requests made */
	unsigned int	kb_in, kb_out;		/* IN/OUT traffic counters */
	struct mimetype	*mimetypes;		/* Known mime types */
	struct mountpoint *mountpoints;		/* Aliases */
	struct envvar	*envvars;		/* CGI environment variables */
	struct userurl	*urls;			/* User urls */
	struct userauth	*auths;			/* User auth files */
	struct conn	*connections;		/* List of connections */
#ifdef HAVE_SSL
	SSL_CTX		*ssl_ctx;		/* SSL context */
#endif
	/*
	 * Configurable
	 */
	FILE		*accesslog;		/* Access log stream */
	FILE		*errorlog;		/* Error log stream */
	char		*put_auth;		/* PUT auth file */
	char		root[FILENAME_MAX];	/* Document root */
	char		*index;			/* Index files */
	char		*ext;			/* CGI extention */
	char		*interp;		/* CGI script interpreter */
	char		*realm;			/* Auth realm */
	char		*pass;			/* Global passwords file */
	char		*uid;			/* Run as user */
	int		port;			/* Listening port */
	int		dirlist;		/* Directory listing */
	int		_debug;			/* Debug flag */
	int		gui;			/* Show GUI flag */
	shttpd_bauth_callback_t         bauthf; /* basic authorization callback */
	shttpd_dauth_callback_t         dauthf; /* digest authorization callback */
};

typedef void (*optset_t)(struct shttpd_ctx *, void *ptr, const char *string);
struct opt {
	int		sw;			/* Command line switch */
	const char	*name;			/* Option name in config file */
	const char	*desc;			/* Description */
	optset_t	setter;			/* Option setter function */
	size_t		ofs;			/* Value offset in context */
	const char	*arg;			/* Argument format */
	const char	*def;			/* Default option value */
	char		*tmp;			/* Temporary storage */
	unsigned int	flags;			/* Flags */
#define	OPT_FLAG_BOOL	1
#define	OPT_FLAG_INT	2
#define	OPT_FLAG_FILE	4
#define	OPT_FLAG_DIR	8
};

/*
 * Global variables
 */
static time_t		current_time;	/* No need to keep per-context time */
static int		_debug;		/* Show _debug messages */
static int		exit_flag;	/* Exit flag */
static int		inetd;		/* Inetd flag */

/*
 * Prototypes
 */
static void	io_inc_tail(struct io *io, size_t n);
static int	casecmp(register const char *s1, register const char *s2);
static int	ncasecmp(register const char *, register const char *, size_t);
static char	*mystrdup(const char *str);
static void	disconnect(struct conn *c);
static int	writeremote(struct conn *c, const char *buf, size_t len);
static int	readremote(struct conn *c, char *buf, size_t len);
static int	nonblock(int fd);
static int	getreqlen(const char *buf, size_t buflen);
static void	log_access(FILE *fp, const struct conn *c);
static void	senderr(struct conn *c, int status, const char *descr,
		const char *headers, const char *fmt, ...);
static int	montoi(const char *s);
static time_t	datetosec(const char *s);
static void	urldecode(char *from, char *to);
static void	killdots(char *file);
static char	*fetch(const char *src, char *dst, size_t len);
static void     parse_headers(struct conn *c, char *s);
static void	parse_request(struct conn *c);
static void	handle(struct conn *c);
static void	serve(struct shttpd_ctx *,void *);
static void	mystrlcpy(register char *, register const char *, size_t);
static struct opt *swtoopt(int sw, const char *name);
static struct shttpd_ctx *do_init(const char *, int argc, char *argv[]);

static void
add_conn_to_ctx(struct shttpd_ctx *ctx, struct conn *c)
{
	EnterCriticalSection(&ctx->mutex);

	c->next = ctx->connections;
	ctx->connections = c;
	ctx->nactive++;
	c->ctx = ctx;

	LeaveCriticalSection(&ctx->mutex);
}

static void
del_conn_from_ctx(struct shttpd_ctx *ctx, struct conn *c)
{
	struct conn	*tmp;

	EnterCriticalSection(&ctx->mutex);

	if (c == ctx->connections)
		ctx->connections = c->next;
	else
		for (tmp = ctx->connections; tmp != NULL; tmp = tmp->next)
			if (tmp->next == c) {
				tmp->next = c->next;
				break;
			}

	ctx->nactive--;
	assert(ctx->nactive >= 0);

	LeaveCriticalSection(&ctx->mutex);
}

const char *
shttpd_version(void)
{
	return (SHTTPD_VERSION);
}

/*
 * Add mime type to the known mime type list
 */
void
shttpd_addmimetype(struct shttpd_ctx *ctx, const char *ext, const char *mime)
{
	struct mimetype	*p;

	/* XXX possible resource leak  */
	if ((p = calloc(1, sizeof(*p))) != NULL &&
			(p->ext = mystrdup(ext)) != NULL &&
			(p->mime = mystrdup(mime)) != NULL) {
		p->extlen	= strlen(p->ext);
		p->next		= ctx->mimetypes;
		ctx->mimetypes	= p;
	}
}

/*
 * Configuration parameters setters
 */
static void
set_int(struct shttpd_ctx *ctx, void *ptr, const char *string)
{
	ctx = NULL;	/* Unused */
	* (int *) ptr = atoi(string);
}

static void
set__debug(struct shttpd_ctx *ctx, void *ptr, const char *string)
{
	ctx = ptr = NULL;
	_debug = atoi(string);
}

static void
set_inetd(struct shttpd_ctx *ctx, void *ptr, const char *string)
{
	ctx = ptr = NULL;
	inetd = atoi(string);
}

static void
set_str(struct shttpd_ctx *ctx, void *ptr, const char *string)
{
	ctx = NULL;	/* Unused */
	* (char **) ptr = strdup(string);
}

static void
set_root(struct shttpd_ctx *ctx, void *ptr, const char *string)
{
	mystrlcpy(ptr, string, sizeof(ctx->root));
}

static void
set_access_log(struct shttpd_ctx *ctx, void *ptr, const char *string)
{
	FILE	*fp;

	assert(&ctx->accesslog == ptr);

	if ((fp = fopen(string, "a")) == NULL)
		elog(ERR_INFO, "cannot open log file %s: %s",
				string, strerror(errno));
	else
		ctx->accesslog = fp;
}

static void
set_error_log(struct shttpd_ctx *ctx, void *ptr, const char *string)
{
	ptr = NULL;
	ctx = NULL;	/* Unused */
	if (freopen(string, "w", stderr) == NULL)
		elog(ERR_INFO, "freopen(%s): %s", string, strerror(errno));
}

static void
set_mime(struct shttpd_ctx *ctx, void *arg, const char *string)
{
	FILE	*fp;
	char	line[512], ext[sizeof(line)], mime[sizeof(line)], *s;
	static struct { const char *ext, *type; } *p, types[] = {
		{"svg",		"image/svg+xml"			},
		{"torrent",	"application/x-bittorrent"	},
		{"wav",		"audio/x-wav"			},
		{"mp3",		"audio/x-mp3"			},
		{"mid",		"audio/mid"			},
		{"m3u",		"audio/x-mpegurl"		},
		{"ram",		"audio/x-pn-realaudio"		},
		{"ra",		"audio/x-pn-realaudio"		},
		{"doc",		"application/msword",		},
		{"exe",		"application/octet-stream"	},
		{"zip",		"application/x-zip-compressed"	},
		{"xls",		"application/excel"		},
		{"tgz",		"application/x-tar-gz"		},
		{"tar.gz",	"application/x-tar-gz"		},
		{"tar",		"application/x-tar"		},
		{"gz",		"application/x-gunzip"		},
		{"arj",		"application/x-arj-compressed"	},
		{"rar",		"application/x-arj-compressed"	},
		{"rtf",		"application/rtf"		},
		{"pdf",		"application/pdf"		},
		{"jpeg",	"image/jpeg"			},
		{"png",		"image/png"			},
		{"mpg",		"video/mpeg"			},
		{"mpeg",	"video/mpeg"			},
		{"asf",		"video/x-ms-asf"		},
		{"avi",		"video/x-msvideo"		},
		{"bmp",		"image/bmp"			},
		{"jpg",		"image/jpeg"			},
		{"gif",		"image/gif"			},
		{"ico",		"image/x-icon"			},
		{"txt",		"text/plain"			},
		{"css",		"text/css"			},
		{"htm",		"text/html"			},
		{"html",	"text/html"			},
		{NULL,		NULL				}
	};

	arg = NULL;

	if (strcmp(string, "builtin") == 0) {
		for (p = types; p->ext != NULL; p++)
			shttpd_addmimetype(ctx, p->ext, p->type);
		return;
	}

	if ((fp = fopen(string, "r")) == NULL)
		elog(ERR_FATAL, "setmimetypes: fopen(%s): %s",
				string, strerror(errno));

	while (fgets(line, sizeof(line), fp) != NULL) {
		/* Skip empty lines */
		if (line[0] == '#' || line[0] == '\n')
			continue;
		if (sscanf(line, "%s", mime)) {
			s = line + strlen(mime);
			while (*s && *s != '\n' && sscanf(s, "%s", ext)) {
				shttpd_addmimetype(ctx, ext, mime);
				s += strlen(mime);
			}
		}
	}

	(void) fclose(fp);
}

#ifdef HAVE_SSL
static void
set_ssl(struct shttpd_ctx *ctx, void *arg, const char *pem)
{
	SSL_CTX		*CTX;
	//	void		*lib;
	//	struct ssl_func	*fp;

	arg = NULL;	/* Unused */
#if 0
	/* Load SSL library dynamically */
	if ((lib = dlopen(SSL_LIB, RTLD_LAZY)) == NULL) {
		elog(ERR_FATAL, "set_ssl: cannot load %s", SSL_LIB);
	}

	for (fp = ssl_sw; fp->name != NULL; fp++)
		if ((fp->ptr.value_void = dlsym(lib, fp->name)) == NULL)
			elog(ERR_FATAL, "set_ssl: cannot find %s", fp->name);
#endif

	/* Initialize SSL crap */
	SSL_library_init();

	if ((CTX = SSL_CTX_new(SSLv23_server_method())) == NULL)
		elog(ERR_FATAL, "SSL_CTX_new error");
	else if (SSL_CTX_use_certificate_file(CTX, pem, SSL_FILETYPE_PEM) == 0)
		elog(ERR_FATAL, "cannot open certificate %s", pem);
	ctx->ssl_ctx = CTX;
}

static void
set_ssl_priv_key(struct shttpd_ctx *ctx, void *arg, const char *pem)
{
	if (ctx->ssl_ctx == NULL) {
		elog(ERR_FATAL, "SSL_CTX is not created");
	}
	if (SSL_CTX_use_PrivateKey_file(ctx->ssl_ctx, pem, SSL_FILETYPE_PEM) == 0) {
		elog(ERR_FATAL, "cannot open private key %s", pem);
	}
}
#endif

/*
 * Setup aliases. Passed string must be in format
 * "directory1=url1,directory2=url2,..."
 */
static void
set_aliases(struct shttpd_ctx *ctx, void *arg, const char *string)
{
	struct mountpoint	*mp, **head = arg;
	char	*p, *s, *copy, dir[FILENAME_MAX], uri[IO_MAX];

	ctx = NULL;	/* Unused */

	for (s = p = copy = mystrdup(string); *s; s = p) {
		if ((p = strchr(s, ',')) != NULL)
			*p++ = '\0';
		else
			p = s + strlen(s);

		/* FIXME: overflow possible */
		if (sscanf(s, "%[^=]=%s", dir, uri) != 2)
			elog(ERR_DEBUG, "set_aliases: bad format: %s", string);
		else if ((mp = calloc(1, sizeof(*mp))) != NULL) {
			mp->mountpoint	= mystrdup(uri);
			mp->path	= mystrdup(dir);
			mp->next	= *head;
			*head		= mp;
			elog(ERR_DEBUG, "set_aliases: [%s]=[%s]", dir, uri);
		}
	}

	free(copy);
}

/*
 * Setup CGI environment variables. Passed string must be in format
 * "varname1=value1,varname2=value2,..."
 */
static void
set_envvars(struct shttpd_ctx *ctx, void *arg, const char *string)
{
	struct envvar	*ep, **head = arg;
	char	*p, *s, *copy, name[IO_MAX], value[IO_MAX];

	ctx = NULL;	/* Unused */

	for (s = p = copy = mystrdup(string); *s; s = p) {
		if ((p = strchr(s, ',')) != NULL)
			*p++ = '\0';
		else
			p = s + strlen(s);

		/* FIXME: overflow possible */
		if (sscanf(s, "%[^=]=%s", name, value) != 2)
			elog(ERR_DEBUG, "set_envvars: bad format: %s", string);
		else if ((ep = calloc(1, sizeof(*ep))) != NULL) {
			ep->name	= mystrdup(name);
			ep->value	= mystrdup(value);
			ep->next	= *head;
			*head		= ep;
			elog(ERR_DEBUG, "set_envvars: [%s]=[%s]", name, value);
		}
	}

	free(copy);
}

#define	OFS(x)	offsetof(struct shttpd_ctx, x)
#define BOOL_OPT	"0|1"
static struct opt options[] = {
	{'d', "document_root", "Web root directory", set_root,
		OFS(root), "directory", NULL, NULL, OPT_FLAG_DIR},
	{'i', "index_files", "Index files", set_str,
		OFS(index), "file_names", INDEX_FILES, NULL, 0	},
	{'p', "listen_port", "Listening port", set_int,
		OFS(port), "port", PORT, NULL, OPT_FLAG_INT},
	{'D', "list_directories", "Directory listing", set_int,
		OFS(dirlist), BOOL_OPT, "1", NULL, OPT_FLAG_BOOL},
	{'c', "cgi_extention", "CGI extention", set_str,
		OFS(ext), "extention", CGI_EXT, NULL, 0		},
	{'C', "cgi_interpreter", "CGI interpreter", set_str,
		OFS(interp), "file", NULL, NULL, OPT_FLAG_FILE	},
	{'N', "auth_realm", "Authentication realm", set_str,
		OFS(realm), "auth_realm", REALM, NULL, 0	},
	{'l', "access_log", "Access log file", set_access_log,
		OFS(accesslog), "file", NULL, NULL, OPT_FLAG_FILE},
	{'e', "error_log", "Error log file", set_error_log,
		OFS(errorlog), "file", NULL, NULL, OPT_FLAG_FILE},
	{'m', "mime_types", "Mime types file", set_mime,
		0, "file", "builtin", NULL, OPT_FLAG_FILE	},
	{'P', "global_htpasswd", "Global passwords file", set_str,
		OFS(pass), "file", NULL, NULL, OPT_FLAG_FILE	},
	{'v', "debug", "Debug mode", set__debug,
		0, BOOL_OPT, "0", NULL, OPT_FLAG_BOOL	},
#ifdef HAVE_SSL
	{'s', "ssl_certificate", "SSL certificate file", set_ssl,
		OFS(ssl_ctx), "pem_file", NULL, NULL, OPT_FLAG_FILE},
	{'k', "ssl_priv_key", "SSL pivate key file", set_ssl_priv_key,
		OFS(ssl_ctx), "pem_file", NULL, NULL, OPT_FLAG_FILE},
#endif
	{'U', "put_auth", "PUT,DELETE auth file",set_str,
		OFS(put_auth), "file", NULL, NULL, OPT_FLAG_FILE},
	{'V', "cgi_envvar", "CGI envir variables", set_envvars,
		OFS(envvars), "X=Y,....", NULL, NULL, 0		},
	{'a', "aliases", "Aliases", set_aliases,
		OFS(mountpoints), "X=Y,...", NULL, NULL, 0	},
	{'I', "inetd_mode", "Inetd mode", set_inetd,
		0, BOOL_OPT, "0", NULL, OPT_FLAG_BOOL	},
	{'u', "runtime_uid", "Run as user", set_str,
		OFS(uid), "user_name", NULL, NULL, 0		},
	{0,   NULL, NULL, NULL, 0, NULL, NULL, NULL, 0		}
};

static struct userurl *
isregistered(struct shttpd_ctx *ctx, const char *url)
{
	struct userurl	*p;

	for (p = ctx->urls; p != NULL; p = p->next)
		if (strcmp(p->url, url) == 0)
			return (p);

	return (NULL);
}


#ifdef EMBEDDED
/*
 * The URI should be handled is user-registered callback function.
 * In MT scenario, call user-defined function in dedicated thread (or process)
 * and discard the return value.
 * In nonMT scenario, call the user function. Return value should is the
 * number of bytes copied to the local IO buffer. Mark local IO as done,
 * and shttpd will take care about passing data back to client.
 */
static void
do_embedded(struct conn *c)
{
	struct shttpd_arg_t	arg;
	const struct userurl	*url = c->userurl;
	unsigned long		n;

	arg.priv	= c;
	arg.last	= 0;
	arg.state	= c->state;
	arg.user_data	= url->data;
	arg.buf		= c->local.buf + c->local.head;
	arg.buflen	= IO_SPACELEN(&c->local);

	if (c->http_method == METHOD_POST && c->cclength) {

		if (c->query == NULL) {

			/* Allocate POST buffer */
			if ((c->query = malloc(c->cclength + 1)) == NULL) {
				senderr(c, 413, "Too Large", "", "huge POST");
				return;
			}

			/* Copy initial POST data into c->query */
			n = c->remote.head - c->reqlen;
			if ((size_t) n > c->cclength)
				n = c->cclength;
			if (n > 0) {
				(void) memcpy(c->query,
						c->remote.buf + c->reqlen, n);
				c->nposted += n;
			}
			c->remote.head = c->remote.tail = 0;
			elog(ERR_DEBUG, "do_embedded 1: %u %u",
					c->cclength, c->nposted);
		} else {
			/* Buffer in POST data */
			n = IO_DATALEN(&c->remote);
			if (n > c->cclength - c->nposted)
				n = c->cclength - c->nposted;
			if (n > 0) {
				(void) memcpy(c->query + c->nposted,
						c->remote.buf + c->remote.tail, n);
				c->nposted += n;
			}
			c->remote.head = c->remote.tail = 0;
			elog(ERR_DEBUG, "do_embedded 2: %u %u",
					c->cclength, c->nposted);
		}

		/* Return if not all POST data buffered */
		if (c->nposted < c->cclength || c->cclength == 0) {
			elog(ERR_DEBUG,"do_embedded: c->nposted = %d; cclength = %d",
					c->nposted, c->cclength);
			return;
		}
		/* Null-terminate query data */
		c->query[c->cclength] = '\0';
	}
#if 0
	printf("         c->query body\n");
	char *b = c->query;
	int i, slen;
	while (b < c->query + c->cclength) {
		slen = strlen(b);
		printf("   %s\n", b);
		b += slen + 1;
	}
	printf("         end of c->query body\n");
#endif

	/* Now, when all POST data is read, we can call user callback */
	c->local.head += url->func(&arg);
	c->state = arg.state;
	assert(c->local.head <= c->local.bufsize);

	if (arg.last) {
		c->local.done++;
		c->io = NULL;
		c->flags &= ~FLAG_HAVE_TO_WRITE;
	} else {
		c->flags |= FLAG_HAVE_TO_WRITE;
		// debug("c->flags |= FLAG_HAVE_TO_WRITE");
	}
}

const char *
shttpd_get_header(struct shttpd_arg_t *arg, const char *header_name)
{
	struct conn	*c = arg->priv;
	size_t		len;
	char		*p, *s, *e;

	p = strchr(c->saved, '\n') + 1;
	e = c->saved + c->reqlen;

	len = strlen(header_name);

	while (p < e) {
		if ((s = strchr(p, '\n')) != NULL)
			s[s[-1] == '\r' ? -1 : 0] = '\0';
		if (ncasecmp(header_name, p, len) == 0)
			return (p + len + 2);

		p += strlen(p) + 1;
	}

	return (NULL);
}


static void
free_headers_hnode(hnode_t *n, void *arg) {
	u_free(n->hash_key);
	u_free(n);
}

hash_t *
shttpd_get_all_headers(struct shttpd_arg_t *arg)
{
	struct conn *c = arg->priv;
	char        *p, *s, *e;
	char *name, *value;
	hash_t *hash = hash_create(HASHCOUNT_T_MAX, 0, 0);

	hash_set_allocator(hash, NULL, free_headers_hnode, NULL);

	p = strchr(c->saved, '\n') + 1;
	e = c->saved + c->reqlen;
	while (p < e) {
		name = p;
		if ((s = strchr(p, '\n')) != NULL) {
			if (s > p && s[-1] == '\r') {
				s[-1] = '\0';
			}
			*s = '\0';
			p = s + 1;
		} else {
			p += strlen(p);
		}
		if ((p < e) && (*p == '\0')) {
			p++;
		}
		value = strchr(name, ':');
		if ((value == NULL) || ((value + 2) >= e) ) {
			continue;
		}
		*value = '\0';
		name = strdup(name);
		*value = ':';
		debug("%s: %s", name, value + 2);
		if (!hash_alloc_insert(hash, name, value + 2)) {
			u_free(name);
		}
	}

	return hash;
}



const char *
shttpd_get_env(struct shttpd_arg_t *arg, const char *env_name)
{
	struct conn	*c = arg->priv;

	if (strcmp(env_name, "REQUEST_METHOD") == 0)
		return (c->method);
	else if (strcmp(env_name, "REMOTE_USER") == 0)
		return (c->user);
	else if (strcmp(env_name, "REMOTE_ADDR") == 0)
		return (inet_ntoa(c->sa.u.sin.sin_addr));/* FIXME NOT MT safe */

	return (NULL);
}

void
shttpd_register_url(struct shttpd_ctx *ctx,
		const char *url, shttpd_callback_t cb,
		int authnotneeded, void *data)
{
	struct userurl	*p;

	if ((p = calloc(1, sizeof(*p))) != NULL) {
		p->func		= cb;
		p->data		= data;
		p->authnotneeded = authnotneeded;
		p->url		= mystrdup(url);
		p->next		= ctx->urls;
		ctx->urls	= p;
	}
}

static void
setopt(const char *var, const char *val)
{
	struct opt	*opt; 

	if ((opt = swtoopt(0, var)) == NULL)
		elog(ERR_FATAL, "setopt: unknown variable [%s]", var);

	if (opt->tmp != NULL)
		free(opt->tmp);
	opt->tmp = mystrdup(val);
}



struct shttpd_ctx *
shttpd_init(const char *config_file, ...)
{
	va_list		ap;
	const char	*opt_name, *opt_value;

	va_start(ap, config_file);
	while ((opt_name = va_arg(ap, const char *)) != NULL) {
		opt_value = va_arg(ap, const char *);
		setopt(opt_name, opt_value);
	}
	va_end(ap);
	(void) signal(SIGPIPE, SIG_IGN);

	return (do_init(config_file, 0, NULL));
}

void
shttpd_protect_url(struct shttpd_ctx *ctx, const char *url, const char *file)
{
	struct userauth	*p;

	if ((p = calloc(1, sizeof(*p))) != NULL) {
		p->url		= mystrdup(url);
		p->filename	= mystrdup(file);
		p->next		= ctx->auths;
		ctx->auths	= p;
	}
}

const char *
shttpd_get_var(struct shttpd_arg_t *arg, const char *var)
{
	struct conn	*c = arg->priv;
	char		*p, *s, *e;
	size_t		len, i;

	if ((p = c->query) == NULL)
		return (NULL);

	/* If the buffer has not been scanned yet, do it now */
	if (c->vars[0] == NULL)
		for (i = 0; *p && i < NELEMS(c->vars); i++, p = e) {
			c->vars[i] = p;
			if ((s = strchr(p, '=')) == NULL)
				break;
			*s++ = '\0';
			if ((e = strchr(s, '&')) != NULL)
				*e++ = '\0';
			else
				e = s + strlen(s);
			urldecode(s, s);
		}

	/* Now, loop over all variables, find the right one, return value */
	len = strlen(var);
	for (i = 0; i < NELEMS(c->vars) && c->vars[i] != NULL; i++)
		if (memcmp(var, c->vars[i], len) == 0)
			return (c->vars[i] + len + 1);

	return (NULL);
}

int
shttpd_get_post_query_len(struct shttpd_arg_t *arg)
{
	struct conn	*c = arg->priv;
	return c->cclength;
}


char *
shttpd_get_post_query(struct shttpd_arg_t *arg)
{
	struct conn	*c = arg->priv;
	return c->query;
	/*
	   char 		*p = c->query;
	   int len;

	   if (p == NULL) {
	   return -1;
	   }
	   if (length > c->cclength) {
	   len = c->cclength;
	   } else {
	   len = length;
	   }

	   memcpy(to, p, len);

	   return len;
	   */
}

int
shttpd_get_http_version(struct shttpd_arg_t *arg)
{
	struct conn	*c = arg->priv;
	char		b[PROTO_SIZE];
	char		*p, *q;
	int ver = 0, rev = 0;

	p = b;
	q = c->proto;
	while(*q) {
		*p++ = toupper(*q++);
	}
	*p = 0;

	sscanf(b, "HTTP/%d.%d", &rev, &ver);

	return ver;
}







const char *
shttpd_get_uri(struct shttpd_arg_t *arg)
{
	struct conn	*c = arg->priv;
	return c->uri;
}


#else
static void do_embedded(struct conn *c) { c->local.done++; }
#endif /* EMBEDDED */

/*
 * strncpy() variant, that always nul-terminates destination.
 */
static void
mystrlcpy(register char *dst, register const char *src, size_t n)
{
	for (; *src != '\0' && n > 1; n--)
		*dst++ = *src++;
	*dst = '\0';
}

/*
 * Sane snprintf(). Acts like snprintf(), but never return -1 or the
 * value bigger than supplied buffer.
 * Thanks Adam Zeldis to pointing snprintf()-caused vulnerability
 * in his audit report.
 */
static int
Snprintf(char *buf, size_t buflen, const char *fmt, ...)
{
	va_list		ap;
	int		n;


	if (buflen == 0)
		return (0);

	va_start(ap, fmt);
	n = vsnprintf(buf, buflen, fmt, ap);
	va_end(ap);

	if (n < 0 || n > (int) buflen - 1) {
		elog(ERR_DEBUG, "vsnprintf returned %d (%s)", n, fmt);
		n = buflen - 1;
	}
	buf[n] = '\0';

	return (n);
}

/*
 * Increment tail counter. If it becomes == head, flush both to 0
 */
static void
io_inc_tail(struct io *io, size_t n)
{
	// debug("tail = %d; head = %d", io->tail,io->head);
	assert(io->tail <= io->head);
	assert(io->head <= io->bufsize);
	io->tail += n;
	assert(io->tail <= io->head);
	if (io->tail == io->head)
		io->head = io->tail = 0;
}


/*
 * Case-insensitive string comparison, a-la strcmp()
 */
static int
casecmp(register const char *s1, register const char *s2)
{
	for (; *s1 != '\0' && *s2 != '\0'; s1++, s2++)
		if (tolower(*s1) != tolower(*s2))
			break;

	return (*s1 - *s2);
}

/* Case insensitive memory comparison, strncasecmp() */
static int
ncasecmp(register const char *s1, register const char *s2, size_t len)
{
	register const char	*e = s1 + len - 1;
	int			ret;

	for (; s1 < e && *s1 != '\0' && *s2 != '\0' &&
			tolower(*s1) == tolower(*s2); s1++, s2++) ;
	ret = tolower(*s1) - tolower(*s2);

	return (ret);
}

/* strdup() is not standard, define it here */
static char *
mystrdup(const char *str)
{
	size_t	len;
	char	*p;

	len = strlen(str);
	if ((p = malloc(len + 1)) != NULL)
		(void) strcpy(p, str);

	return (p);
}

/*
 * Disconnect from remote side, free resources
 */


static void
free_parsed_data(struct conn *c)
{
	/* If parse_headers() allocated any data, free it */
	if (c->useragent) {
		free(c->useragent);
		c->useragent = NULL;
	}
	if (c->user) {
		free(c->user);
		c->user = NULL;
	}
	if (c->cookie) {
		free(c->cookie);
		c->cookie = NULL;
	}
	if (c->ctype) {
		free(c->ctype);
		c->ctype = NULL;
	}
	if (c->referer) {
		free(c->referer);
		c->referer = NULL;
	}
	if (c->location) {
		free(c->location);
		c->location = NULL;
	}
	if (c->auth) {
		free(c->auth);
		c->auth = NULL;
	}
	if (c->path) {
		free(c->path);
		c->path = NULL;
	}

	c->nposted = 0;
	if (c->range) {
		free(c->range);
		c->range = NULL;
	}

	if (c->query) {
		free(c->query);
		c->query = NULL;
	}
	c->nposted = 0;
	c->cclength = 0;
}



static void
disconnect(struct conn *c)
{
	int keep_alive = (c->flags & FLAG_KEEP_CONNECTION);

	elog(ERR_DEBUG, "disconnecting %p", c);

	free_parsed_data(c);

	if (c->remote.buf) {
		free(c->remote.buf);
		c->remote.buf = NULL;
	}
	c->remote.head = 0;
	c->remote.tail = 0;
	c->remote.done = 0;

	if (c->local.buf) {
		free(c->local.buf);
		c->local.buf = NULL;
	}
	c->local.head = 0;
	c->local.tail = 0;
	c->local.done = 0;
	if (keep_alive) {
		c->flags &= (FLAG_SSLACCEPTED | FLAG_AUTHORIZED);
		c->expire = current_time + KEEP_ALIVE_TIME;
		elog(ERR_DEBUG, "connection %p is keeping alive for %d secs",
				c, KEEP_ALIVE_TIME);
		return;
	}
	del_conn_from_ctx(c->ctx, c);

	if (c->ctx->accesslog != NULL)
		log_access(c->ctx->accesslog, c);

	/* In inetd mode, exit if request is finished. */
	if (c->usr) {
		free(c->usr);
		c->usr = NULL;
	}
	if (c->pwd) {
		free(c->pwd);
		c->pwd = NULL;
	}
	if (inetd)
		exit_flag++;
	/* Free resources */
	if (c->fd != -1) {
		if (c->flags & FLAG_CGI)
			(void) closesocket(c->fd);
		else
			(void) close(c->fd);
	}
	if (c->dirp)		(void) closedir(c->dirp);
#ifdef HAVE_SSL
	if (c->ssl)		SSL_free(c->ssl);
#endif
	(void) shutdown(c->sock, 2);
	(void) closesocket(c->sock);
	free(c);
}

#ifdef HAVE_SSL
/*
 * Perform SSL handshake
 */
static void
handshake(struct conn *c)
{
	int	n;
	if ((n = SSL_accept(c->ssl)) == 0) {
		n = SSL_get_error(c->ssl, n);
		if (n != SSL_ERROR_WANT_READ && n != SSL_ERROR_WANT_WRITE)
			c->flags |= FLAG_FINISHED;
		elog(ERR_INFO, "handshake: SSL_accept error %d", n);
	} else {
		elog(ERR_DEBUG, "handshake: SSL accepted. sock %d", c->sock);
		c->flags |= FLAG_SSLACCEPTED;
	}
}
#endif

#define	INCREMENT_KB(nbytes, static_counter, kbytes)		\
	do {								\
		static_counter += nbytes;				\
		if (static_counter > 1024) {				\
			kbytes += static_counter / 1024;		\
			static_counter %= 1024;				\
		}							\
	} while (0)

/*
 * Send data to a remote end. Return bytes sent.
 */
static int
writeremote(struct conn *c, const char *buf, size_t len)
{
	static int	out;
	int		n;

	/* Make sure we will not send more than needed */
	if (c->sclength && c->nsent + len > c->sclength + c->shlength)
		len = c->sclength + c->shlength - c->nsent;

	/* Send the data via socket or SSL connection */
#ifdef HAVE_SSL
	if (c->ssl) {
		n = SSL_write(c->ssl, buf, len);
	}
	else 
#endif
		n = send(c->sock, buf, len, 0);

	if (n > 0) {
		c->nsent += n;
		INCREMENT_KB(n, out, c->ctx->kb_out);
	}

	if (n == 0 || (n < 0 && ERRNO != EWOULDBLOCK) ||
			(c->sclength > 0 && c->nsent >= c->sclength + c->shlength)) {
		c->remote.done = 1;
	}

	elog(ERR_DEBUG, "writeremote: %d %d %u %u %u %d [%d: %s]",
			n, len, c->nsent, c->sclength, c->shlength,
			c->remote.done, errno, strerror(errno));

	return (n);
}

/*
 * Read data from the remote end. Return bytes read.
 */
static int
readremote(struct conn *c, char *buf, size_t len)
{
	static int	in;
	int		n = -1;
#ifdef HAVE_SSL
	int ssl_err;
	if (c->ssl) {
		if (!(c->flags & FLAG_SSLACCEPTED)) {
			handshake(c);
		}
		if (c->flags & FLAG_FINISHED) {
			return -1;
		}
		n = SSL_read(c->ssl, buf, len);
		if (n < 0) {
			ssl_err = SSL_get_error(c->ssl, n);
			if (ssl_err == SSL_ERROR_WANT_READ) {
				return -1;
			}
		}
	} else
#endif
		n = recv(c->sock, buf, len, 0);
	if (n > 0)
		INCREMENT_KB(n, in, c->ctx->kb_in);

	if (n == 0 || (n < 0 &&
				((ERRNO != EWOULDBLOCK))))
		c->remote.done = 1;
	return (n);
}


/*
 * Put given file descriptor in blocking (block == 1)
 * or non-blocking (block == 0) mode.
 * Return 0 if success, or -1
 */
static int
nonblock(int fd)
{
	int	ret = -1;
	int	flags;

	if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
		elog(ERR_INFO, "nonblock: fcntl(F_GETFL): %d", ERRNO);
	else if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0)
		elog(ERR_INFO, "nonblock: fcntl(F_SETFL): %d", ERRNO);
	else
		ret = 0;	/* Success */

	return (ret);
}

/*
 * Setup listening socket on given port, return socket
 */
int
shttpd_open_port(int port)
{
	int		sock, on = 1;
	struct usa	sa;


	sa.len				= sizeof(sa.u.sin);
	sa.u.sin.sin_family		= AF_INET;
	sa.u.sin.sin_port		= htons((uint16_t) port);
	sa.u.sin.sin_addr.s_addr	= htonl(INADDR_ANY);

	if ((sock = socket(PF_INET, SOCK_STREAM, 6)) == -1)
		elog(ERR_FATAL, "shttpd_open_port: socket: %s",strerror(ERRNO));
	else if (nonblock(sock) != 0)
		elog(ERR_FATAL, "shttpd_open_port: nonblock");
	else if (setsockopt(sock, SOL_SOCKET,
				SO_REUSEADDR,(char *) &on, sizeof(on)) != 0)
		elog(ERR_FATAL, "shttpd_open_port: setsockopt");
	else if (bind(sock, &sa.u.sa, sa.len) < 0)
		elog(ERR_FATAL, "shttpd_open_port: bind(%d): %s",
				port, strerror(ERRNO));
	else if (listen(sock, 128) != 0)
		elog(ERR_FATAL, "shttpd_open_port: listen: %s",strerror(ERRNO));
	(void) fcntl(sock, F_SETFD, FD_CLOEXEC);

	return (sock);
}

/*
 * Check whether full request is buffered
 * Return request length, or 0
 */
static int
getreqlen(const char *buf, size_t buflen)
{
	const char	*s, *e;
	int		len = 0;

	for (s = buf, e = s + buflen - 1; len == 0 && s < e; s++)
		if (!isprint(*(unsigned char *) s) && *s != '\r' && *s != '\n')
			len = -1;
		else if (s[0] == '\n' && s[1] == '\n')
			len = s - buf + 2;
		else if (s[0] == '\n' && &s[1] < e &&
				s[1] == '\r' && s[2] == '\n')
			len = s - buf + 3;

	return (len);
}

/*
 * Write an HTTP access log into a file `logfile'
 */
static void
log_access(FILE *fp, const struct conn *c)
{
	char	date[64];

	strftime(date, sizeof(date), "%d/%b/%Y %H:%M:%S", localtime(&current_time));
	(void) fprintf(fp, "%s - %s [%s] \"%s %s %s\" %d %lu ",
			inet_ntoa(c->sa.u.sin.sin_addr), c->user ? c->user : "-",
			date, c->method, c->uri, c->proto, c->status,
			c->nsent > c->shlength ? c->nsent - c->shlength : 0);

	if (c->referer)
		(void) fprintf(fp, "\"%s\"", c->referer);
	else
		(void) fputc('-', fp);
	(void) fputc(' ', fp);

	if (c->useragent)
		(void) fprintf(fp, "\"%s\" ", c->useragent);
	else
		(void) fputc('-', fp);

	(void) fputc('\n', fp);
	(void) fflush(fp);
}

/*
 * Send an error back to a client.
 */
static void
senderr(struct conn *c, int status, const char *descr,
		const char *headers, const char *fmt, ...)
{
	va_list	ap;
	char	msg[c->local.bufsize];
	int	n;

	c->shlength = n = Snprintf(msg, sizeof(msg),
			"HTTP/1.1 %d %s\r\nConnection: close\r\n%s%s\r\n%d ",
			//       "HTTP/1.1 %d %s\r\n%s%s\r\n%d ",
			status, descr, headers, headers[0] == '\0' ? "" : "\r\n", status);
	va_start(ap, fmt);
	n += vsnprintf(msg + n, sizeof(msg) - n, fmt, ap);
	if (n > (int) sizeof(msg))
		n = sizeof(msg);
	va_end(ap);
	mystrlcpy(c->local.buf, msg, c->local.bufsize);
	c->local.head = n;
	c->local.tail = 0;
	c->flags &= ~FLAG_KEEP_CONNECTION;
	elog(ERR_DEBUG, "%s: [%s]", "senderr", c->local.buf);
	c->status = status;
	c->local.done++;
}

/*
 * Convert month to the month number. Return -1 on error, or month number
 */
static int
montoi(const char *s)
{
	static const char *ar[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	size_t	i;

	for (i = 0; i < NELEMS(ar); i++)
		if (casecmp(s, ar[i]) == 0)
			return (i);

	return (-1);
}

/*
 * Parse date-time string, and return the corresponding time_t value
 */
static time_t
datetosec(const char *s)
{
	struct tm	tm;
	char		mon[32];
	int		sec, min, hour, mday, month, year;

	(void) memset(&tm, 0, sizeof(tm));
	sec = min = hour = mday = month = year = 0;

	if (((sscanf(s, "%d/%3s/%d %d:%d:%d",
						&mday, mon, &year, &hour, &min, &sec) == 6) ||
				(sscanf(s, "%d %3s %d %d:%d:%d",
					&mday, mon, &year, &hour, &min, &sec) == 6) ||
				(sscanf(s, "%*3s, %d %3s %d %d:%d:%d",
					&mday, mon, &year, &hour, &min, &sec) == 6) ||
				(sscanf(s, "%d-%3s-%d %d:%d:%d",
					&mday, mon, &year, &hour, &min, &sec) == 6)) &&
			(month = montoi(mon)) != -1) {
		tm.tm_mday	= mday;
		tm.tm_mon	= month;
		tm.tm_year	= year;
		tm.tm_hour	= hour;
		tm.tm_min	= min;
		tm.tm_sec	= sec;
	}

	if (tm.tm_year > 1900)
		tm.tm_year -= 1900;
	else if (tm.tm_year < 70)
		tm.tm_year += 100;

	return (mktime(&tm));
}



/*
 * Decode urlencoded string. from and to may point to the same location
 */
static void
urldecode(char *from, char *to)
{
	int	i, a, b;
#define	HEXTOI(x)  (isdigit(x) ? x - '0' : x - 'W')
	for (i = 0; *from != '\0'; i++, from++) {
		if (from[0] == '%' &&
				isxdigit((unsigned char) from[1]) &&
				isxdigit((unsigned char) from[2])) {
			a = tolower(from[1]);
			b = tolower(from[2]);
			to[i] = (HEXTOI(a) << 4) | HEXTOI(b);
			from += 2;
		} else if (from[0] == '+') {
			to[i] = ' ';
		} else {
			to[i] = from[0];
		}
	}
	to[i] = '\0';
}

/*
 * Protect from document root traversal using "../../"
 */
static void
killdots(char *file)
{
	char	*s = file, *dup = malloc(strlen(file) + 1), *p = dup;

	if (dup == NULL)
		return;

	while (*s != '\0') {
		*p++ = *s++;
		if (s[-1] == '/')
			while (*s == '.' || *s == '/')
				s++;
	}
	*p = '\0';
	(void) strcpy(file, dup);
	free(dup);
}

/*
 * Fetch header value into specified destination
 */
static char *
fetch(const char *src, char *dst, size_t len)
{
	const char	*p;

	*dst = '\0';
	if ((p = strchr(src, '\r')) || (p = strchr(src, '\n'))) {
		len--;	/* For terminating '\0' */
		if (p - src < (int) len)
			len = p - src;
		(void) memcpy(dst, src, len);
		dst[len] = '\0';
	}

	return (dst);
}

/*
 * Parse HTTP headers, filling values in struct conn
 */
static void
parse_headers(struct conn *c, char *s)
{
	char		val[IO_MAX];
	struct header	*header;
	union variant	*v;

	/* Loop through all headers in the request */
	while (s != NULL && s[0] != '\n' && s[0] != '\r') {
		/* Is this header known to us ? */
		for (header = headers; header->len != 0; header++)
			if (ncasecmp(s, header->name, header->len) == 0)
				break;

		/* If the header is known to us, store its value */
		if (header->len != 0) {

			/* Fetch the header value into val */
			fetch(s + header->len, val, sizeof(val));

			/* Find place in a connection structure to store it */
			v = (union variant *) ((char *) c + header->offset);

			/* Fetch header value into the connection structure */
			if (header->type == HDR_STRING) {
				if (v->value_str == NULL)
					v->value_str = mystrdup(val);
			} else if (header->type == HDR_INT) {
				v->value_int = strtoul(val, NULL, 10);
			} else if (header->type == HDR_DATE) {
				v->value_time = datetosec(val);
			}
		}

		/* Shift to the next header */
		if ((s = strchr(s, '\n')) != NULL)
			s++;
	}

	return;

}





/*
 * HTTP digest authentication
 */

void
shttpd_register_bauth_callback(struct shttpd_ctx *ctx,
		shttpd_bauth_callback_t bauthf)
{
	ctx->bauthf = bauthf;
}

void
shttpd_register_dauth_callback(struct shttpd_ctx *ctx,
		shttpd_dauth_callback_t dauthf)
{
	ctx->dauthf = dauthf;
}


static void
fetchfield(const char **from, char *to, int len, int shift)
{
	int		n;
	char		fmt[20];
	const char	*p = *from + shift;

	*from = p;

	if (*p == '"') {
		Snprintf(fmt, sizeof(fmt), "%%%d[^\"]%%n", len - 1);
		p++;
	} else {
		Snprintf(fmt, sizeof(fmt), "%%%d[^ \t,]%%n", len - 1);
	}

	elog(ERR_DEBUG, "fetchfield: [%s] [%s]", fmt, p);

	if (sscanf(p, fmt, to, &n)) {
		p += n;
		*from = p;
	}
}

/*
 * Fetch a password provided by user.
 * Return 1 if success, 0 otherwise.
 */
static int
getauth(struct conn *c, struct digest *dig)
{
	const char	*p = c->auth, *e = p + strlen(c->auth);

	if (ncasecmp(p, "Digest ", 7) != 0)
		return (0);

	(void) memset(dig, 0, sizeof(dig));

	for (p += 7; p < e; p++)
		if (ncasecmp(p, "username=", 9) == 0)
			fetchfield(&p, dig->user, sizeof(dig->user), 9);
		else if (ncasecmp(p, "nonce=", 6) == 0)
			fetchfield(&p, dig->nonce, sizeof(dig->nonce), 6);
		else if (ncasecmp(p, "response=", 9) == 0)
			fetchfield(&p, dig->resp, sizeof(dig->resp), 9);
		else if (ncasecmp(p, "uri=", 4) == 0)
			fetchfield(&p, dig->uri, sizeof(dig->uri), 4);
		else if (ncasecmp(p, "qop=", 4) == 0)
			fetchfield(&p, dig->qop, sizeof(dig->qop), 4);
		else if (ncasecmp(p, "cnonce=", 7) == 0) {
			if (*(p + 7) != '"') {
				return 0;
			}
			char *pp = strchr(p+8, '"');
			if (pp == NULL) {
				return 0;
			}
			size_t L = (pp - p) - 7;
			char *bb = malloc(L + 1);
			if (bb == NULL) {
				return 0;
			}
			fetchfield(&p, bb, L + 1, 7);
			//            p = pp + L;
			dig->cnonce = bb;
		} else if (ncasecmp(p, "nc=", 3) == 0) {
			fetchfield(&p, dig->nc, sizeof(dig->nc), 3);
		}

	elog(ERR_DEBUG, "[%s] [%s] [%s] [%s] [%s] [%s]",
			dig->user, dig->uri, dig->resp, dig->qop, dig->cnonce, dig->nc);
	if (dig->cnonce == NULL) {
		return 0;
	}
	return (1);
}



/*
 * Check authorization. Return 1 if not needed or authorized, 0 otherwise
 */
static int
checkauth(struct conn *c, const char *path)
{
	struct digest	di;
	char *p, *pp;
	int  l;
	int res;
	struct shttpd_ctx *ctx = c->ctx;

	if (c->flags & FLAG_AUTHORIZED) {
		return 1;
	}
	if (!ctx->bauthf && !ctx->dauthf) {
		return 1;
	}
	if (c->auth == NULL) {
		return 0;
	}

	if (ncasecmp(c->auth, "Digest ", 7) == 0) {
		if (ctx->dauthf == NULL) {
			return 0;
		}
		if (getauth(c, &di) == 0) {
			return 0;
		}
		res = ctx->dauthf(ctx->realm, c->method, &di);
		if (res) {
			c->flags |= FLAG_AUTHORIZED;
		}
		u_free(di.cnonce);
		return res;
	}

	if (ncasecmp(c->auth, "Basic ", 6) != 0) {
		return 0;
	}

	if (ctx->bauthf == NULL) {
		return 0;
	}

	p = c->auth + 5;
	while ((*p == ' ') || (*p == '\t')) {
		p++;
	}
	pp = p;
	while ((*p != ' ') && (*p != '\t') && (*p != '\r')
			&& (*p != '\n') && (*p != 0)) {
		p++;
	}

	if (pp == p) {
		return 0;
	}
	*p = 0;

	// use di.uri as a bufer. It's big ehough 
	l = ws_base64_decode(pp, p - pp, di.uri);
	if (l <= 0) {
		return 0;
	}

	di.uri[l] = 0;
	p = di.uri;
	pp = p;
	p = strchr(p, ':');
	if (p == NULL) {
		return 0;
	}
	*p++ = 0;

	res = ctx->bauthf(pp, p);
	if (res) {
		//authorized(net_0)
		c->usr = strdup(pp);
		c->pwd = strdup(p);
		c->flags |= FLAG_AUTHORIZED;
	}

	return res;
}

void      shttpd_get_credentials(struct shttpd_arg_t *arg,
		char **user, char **pwd)
{
	struct conn *c = (struct conn *)arg->priv;

	*user = c->usr;
	*pwd =  c->pwd;
}


static void
send_authorization_request(struct conn *c)
{
	char	buf[512];
	int n = 0;


	if (c->ctx->dauthf) {
		n = Snprintf(buf, sizeof(buf),
				"WWW-Authenticate: Digest qop=\"auth\", realm=\"%s\", "
				"nonce=\"%lu\"", c->ctx->realm, (unsigned long) current_time);
		if (c->ctx->bauthf) {
			n += Snprintf(buf +n, sizeof(buf) - n, "\r\n");
		}
	}
	if (c->ctx->bauthf) {
		(void) Snprintf(buf + n, sizeof(buf) - n,
				"WWW-Authenticate: Basic realm=\"%s\"", c->ctx->realm);
	}
	senderr(c, 401, "Unauthorized", buf, "Authorization required");
	//    c->flags |= FLAG_KEEP_CONNECTION;
}



/*
 * Handle request
 */
static void
handle(struct conn *c)
{
	char			path[FILENAME_MAX + IO_MAX];
	struct userurl		*userurl;
	char *p;

	elog(ERR_DEBUG, "handle: [%s]", c->remote.buf);

	if ((p = strchr(c->uri, '?')) != NULL) {
		*p++ = '\0';
	}
	if (c->http_method != METHOD_POST) {
		senderr(c, 405, "Method not allowed", "", "Must be POST");
		return;
	}

	c->query = NULL;
	urldecode(c->uri, c->uri);
	killdots(c->uri);
	(void) Snprintf(path, sizeof(path), "%s%s", c->ctx->root, c->uri);

	if ((userurl = isregistered(c->ctx, c->uri)) == NULL) {
		senderr(c, 405, "Unknown uri", "", c->uri);
		return;
	}
	if (!userurl->authnotneeded && checkauth(c, path) != 1) {
		send_authorization_request(c);
		return;
	}
	c->userurl = userurl;
	c->io = do_embedded;
	c->flags |= FLAG_ALWAYS_READY;
}

/*
 * Parse HTTP request
 */
static void
parse_request(struct conn *c)
{
	char	fmt[32];
	char    *s = c->remote.buf;


	(void) snprintf(fmt, sizeof(fmt), "%%%us %%%us %%%us",
			(unsigned) sizeof(c->method) - 1, (unsigned) sizeof(c->uri) - 1,
			(unsigned) sizeof(c->proto) - 1);

	/* Get the request line */
	if (sscanf(s, fmt, c->method, c->uri, c->proto) != 3) {
		senderr(c, 404, "Bad Request","", "Bad Request");
	} else if (c->uri[0] != '/') {
		senderr(c, 404, "Bad Request","", "Bad Request");
	} else if (ncasecmp(c->proto, "HTTP", 4) != 0) {
		senderr(c, 501, "Bad Protocol","", "Procotol Not Supported");
	} else if (ncasecmp(c->method, "GET", 3) == 0) {
		c->http_method = METHOD_GET;
	} else if (ncasecmp(c->method, "POST", 4) == 0) {
		c->http_method = METHOD_POST;
	} else if (ncasecmp(c->method, "HEAD", 4) == 0) {
		c->http_method = METHOD_HEAD;
	} else if (ncasecmp(c->method, "PUT", 3) == 0) {
		c->http_method = METHOD_PUT;
	} else if (ncasecmp(c->method, "DELETE", 6) == 0) {
		c->http_method = METHOD_DELETE;
	} else {
		senderr(c, 501, "Not Implemented", "","Method Not Implemented");
	}

	/* XXX senderr() should set c->local.done flag! */
	if (c->local.done != 0) {
		return;
	}
	(void) strcpy(c->saved, s);	/* Save whole request */
	parse_headers(c, strchr(s, '\n') + 1);
	handle(c);
	c->flags |= FLAG_PARSED;
}

/*
 * Process given connection
 */
static void
serve(struct shttpd_ctx *ctx, void *ptr)
{
	struct conn	*c = ptr;
	int		n, len;

	assert(ctx == c->ctx);
	if (c->remote.buf == NULL) {
		c->remote.buf = malloc(c->remote.bufsize);
		if (c->remote.buf == NULL) {
			elog(ERR_DEBUG, "No memory");
			c->flags = FLAG_FINISHED;
			return;
		}
		memset(c->remote.buf, 'b', c->remote.bufsize);
		c->remote.buf[c->remote.bufsize - 1] = 0;
		c->flags |= FLAG_KEEP_CONNECTION;
	}
	if (c->local.buf == NULL) {
		c->local.buf = malloc(c->remote.bufsize);
		if (c->local.buf == NULL) {
			elog(ERR_DEBUG, "No memory");
			c->flags = FLAG_FINISHED;
			return;
		}
		c->flags |= FLAG_KEEP_CONNECTION;
	}

#if 1
	elog(ERR_DEBUG, "serve: enter %p: local %d.%d.%d, remote %d.%d.%d", c,
			c->local.done, c->local.head, c->local.tail,
			c->remote.done, c->remote.head, c->remote.tail);
#endif
	/* Read from remote end */
	assert(c->sock != -1);
	if (c->flags & FLAG_SOCK_READABLE) {
		len = IO_SPACELEN(&c->remote);
		n = readremote(c, c->remote.buf + c->remote.head, len);
		if (n > 0) {
			c->remote.head += n;
			c->remote.buf[c->remote.head] = '\0';
			c->expire += EXPIRE_TIME;
		} else if (!(c->flags & FLAG_PARSED) && c->remote.done) {
			if (n == 0) {
				// connection closed by peer 
				c->flags &= ~FLAG_KEEP_CONNECTION;
			}
			c->flags |= FLAG_FINISHED;
		}
		if (n < 0 ) {
			c->flags &= ~FLAG_KEEP_CONNECTION;
		}
		elog(ERR_DEBUG, "serve: readremote returned %d", n);
	}

	if (c->flags & FLAG_FINISHED) {
		return;
	}

	/* Try to parse the request from remote endpoint */
	if (!(c->flags & FLAG_PARSED)) {
		c->reqlen = getreqlen(c->remote.buf, c->remote.head);
		if (c->reqlen == -1) {
			senderr(c, 400, "Bad Request", "", "Bad request");
		} else if (c->reqlen == 0 && IO_SPACELEN(&c->remote) <= 1) {
			senderr(c, 400, "Bad Request", "","Request is too big");
		} else if (c->reqlen > 0) {
			parse_request(c);
		}
	}

	/* Read from the local endpoint */
	if (!(c->flags & FLAG_FINISHED) && c->io &&
			(c->flags & (FLAG_FD_READABLE | FLAG_SOCK_READABLE |
				     FLAG_HAVE_TO_WRITE))) {
		c->io(c);
		c->expire += EXPIRE_TIME;
	}

	if (c->flags & FLAG_SOCK_WRITABLE) {
		len = IO_DATALEN(&c->local);
		n = writeremote(c, c->local.buf + c->local.tail, len);
		if (n > 0)
			io_inc_tail(&c->local, n);
		elog(ERR_DEBUG, "serve: writeremote returned %d", n);
	}

	if ((c->remote.done && c->remote.head == 0) ||
			(c->local.done && c->local.head == 0)) {
		c->flags |= FLAG_FINISHED;
	}
#if 1
	elog(ERR_DEBUG, "serve: exit %p: local %d.%d.%d, remote %d.%d.%d", c,
			c->local.done, c->local.head, c->local.tail,
			c->remote.done, c->remote.head, c->remote.tail);
#endif
}

void
shttpd_add(struct shttpd_ctx *ctx, int sock)
{
	struct conn	*c;
	struct usa	sa;
#ifdef HAVE_SSL
	SSL		*ssl = NULL;
#endif
	sa.len = sizeof(sa.u.sin);
	(void) nonblock(sock);

	if (getpeername(sock, &sa.u.sa, &sa.len)) {
		elog(ERR_INFO, "shttpd_add: %s", strerror(errno));
#ifdef HAVE_SSL
	} else if (ctx->ssl_ctx && (ssl = SSL_new(ctx->ssl_ctx)) == NULL) {
		elog(ERR_INFO, "shttpd_add: SSL_new: %s", strerror(ERRNO));
		(void) closesocket(sock);
	} else if (ctx->ssl_ctx && SSL_set_fd(ssl, sock) == 0) {
		elog(ERR_INFO, "shttpd_add: SSL_set_fd: %s", strerror(ERRNO));
		(void) closesocket(sock);
		SSL_free(ssl);
#endif
	} else if ((c = calloc(1, sizeof(*c))) == NULL) {
		(void) closesocket(sock);
		elog(ERR_INFO, "shttpd_add: calloc: %s", strerror(ERRNO));
	} else {
		c->sa		= sa;
		c->sock		= sock;
		c->watch	= serve;
		c->watch_data	= c;
		c->fd		= -1;
#ifdef HAVE_SSL
		c->ssl		= ssl;
#endif
		c->birth	= current_time;
		c->expire	= current_time + EXPIRE_TIME;
		c->local.bufsize = IO_MAX;
		c->remote.bufsize = IO_MAX;
		(void) fcntl(sock, F_SETFD, FD_CLOEXEC);
#ifdef HAVE_SSL
		if (ssl)
			handshake(c);
#endif
		ctx->nrequests++;

		add_conn_to_ctx(ctx, c);

		elog(ERR_DEBUG, "shttpd_add: ctx %p, sock %d, conn %p",
				ctx, sock, c);
	}
}

int
shttpd_active(struct shttpd_ctx *ctx)
{
	return (ctx->nactive);
}

static void
do_accept(struct shttpd_ctx *ctx, void *ptr)
{
	int		sock;
	struct usa	sa;

	sa.len = sizeof(sa.u.sin);

	sock = inetd ? fileno(stdin) : accept((int)((char *)ptr - (char *)NULL), &sa.u.sa, &sa.len);	
	if (sock == -1)
		elog(ERR_INFO, "do_accept(%d): %s", (int)((char *)ptr - (char *)NULL), strerror(ERRNO));
	else
		shttpd_add(ctx, sock);
}

/*
 * Setup user watch. If func is NULL, remove it 
 */
void
shttpd_listen(struct shttpd_ctx *ctx, int sock)
{
	struct conn	*c;

	if ((c = calloc(1, sizeof(*c))) == NULL)
		elog(ERR_FATAL, "shttpd_listen: cannot allocate connection");

	c->watch	= do_accept;
	c->watch_data	= (void *)((char *)NULL + sock);
	c->sock		= sock;
	c->fd		= -1;
	c->local.bufsize = IO_MAX;
	c->remote.bufsize = IO_MAX;
	c->expire	= (time_t) LONG_MAX;	/* Never expires */
	add_conn_to_ctx(ctx, c);

	elog(ERR_DEBUG, "shttpd_listen: ctx %p, sock %d, conn %p", ctx,sock,c);
}

int
shttpd_accept(int lsn_sock, int milliseconds)
{
	struct timeval	tv;
	struct usa	sa;
	fd_set		read_set;
	int		sock = -1;

	tv.tv_sec	= milliseconds / 1000;
	tv.tv_usec	= milliseconds % 1000;
	sa.len		= sizeof(sa.u.sin);
	FD_ZERO(&read_set);
	FD_SET(lsn_sock, &read_set);

	if (select(lsn_sock + 1, &read_set, NULL, NULL, &tv) == 1)
		sock = accept(lsn_sock, &sa.u.sa, &sa.len);

	return (sock);
}

/*
 * One iteration of server loop.
 */
void
shttpd_poll(struct shttpd_ctx *ctx, int milliseconds)
{
	struct conn	*c, *nc;
	struct timeval	tv;			/* Timeout for select() */
	fd_set		read_set, write_set;
	int		max_fd = 0, msec = milliseconds;

	current_time = time(0);
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);

	for (c = ctx->connections; c != NULL; c = c->next) {
		c->flags &= ~FLAG_IO_READY;

#define	MERGEFD(fd,set)	\
		do {FD_SET(fd, set); if (fd > max_fd) max_fd = fd;} while (0)

		/* If there is space to read, do it */
		if (IO_SPACELEN(&c->remote))
			MERGEFD(c->sock, &read_set);

		/* If there is data in local buffer, add to write set */
		if ((c->flags & FLAG_HAVE_TO_WRITE) ||
				(IO_DATALEN(&c->local))) {
			elog(ERR_DEBUG, "sock %d ready to write from %d to %d",
					c->sock, c->local.tail, c->local.head);
			MERGEFD(c->sock, &write_set);
		}
#if 0
		/* If not selectable, continue */
		if (c->flags & FLAG_ALWAYS_READY) {
			msec = 0;
		} else {
			/* If there is space left in local buffer, read more */
			if (IO_SPACELEN(&c->local) && c->fd != -1)
				MERGEFD(c->fd, &read_set);

			/* Any data to be passed to the remote end ? */
			if (IO_DATALEN(&c->remote) && c->fd != -1)
				MERGEFD(c->fd, &write_set);
		}
#endif
	}

	tv.tv_sec = msec / 1000;
	tv.tv_usec = msec % 1000;

	/* Check IO readiness */
	if (select(max_fd + 1, &read_set, &write_set, NULL, &tv) < 0) {
		elog(ERR_DEBUG, "select: %s", strerror(errno));
		return;
	}

	/* Set IO readiness flags */
	for (c = ctx->connections; c != NULL; c = c->next) {
		if (FD_ISSET(c->sock, &read_set)) {
			elog(ERR_DEBUG, "%d readable", c->sock);
			c->flags |= FLAG_SOCK_READABLE;
		}
		if (FD_ISSET(c->sock, &write_set)) {
			c->flags |= FLAG_SOCK_WRITABLE;
			//            c->flags &= ~FLAG_HAVE_TO_WRITE;
		}
#if 0
		if (IO_SPACELEN(&c->local) && ((c->flags & FLAG_ALWAYS_READY) ||
					(c->fd != -1 && FD_ISSET(c->fd, &read_set))))
			c->flags |= FLAG_FD_READABLE;
		if (IO_DATALEN(&c->remote) && ((c->flags & FLAG_ALWAYS_READY) ||
					(c->fd != -1 && FD_ISSET(c->fd, &write_set))))
			c->flags |= FLAG_FD_WRITABLE;
#endif
	}

	/* Loop through all connections, handle if IO ready */
	for (c = ctx->connections; c != NULL; c = nc) {
		nc = c->next;

		if (c->flags & FLAG_IO_READY)
			c->watch(c->ctx, c->watch_data);

		if ((c->flags & FLAG_FINISHED) || c->expire < current_time)
			disconnect(c);
	}
}

/*
 * Convert command line switch to the option structure
 */
static struct opt *
swtoopt(int sw, const char *name)
{
	struct opt	*opt;

	for (opt = options; opt->sw != 0; opt++)
		if (sw == opt->sw || (name && strcmp(opt->name, name) == 0))
			return (opt);

	return (NULL);
}

/*
 * Initialize shttpd
 */
struct shttpd_ctx *
do_init(const char *config_file, int argc, char *argv[])
{
	struct shttpd_ctx	*ctx;
	struct opt	*opt;

	current_time = time(NULL);

	if ((ctx = calloc(1, sizeof(*ctx))) == NULL)
		elog(ERR_FATAL, "do_init: cannot allocate context");

	ctx->start_time = current_time;

	/*
	 * setopt() may already set some vars, we do not override them
	 * with the default value if they are set.
	 */
	for (opt = options; opt->sw != 0; opt++)
		if (opt->tmp == NULL && opt->def != NULL)
			opt->tmp = u_strdup(opt->def);

	/* Now, every option must hold its config in 'tmp' node. Call setters */
	for (opt = options; opt->sw != 0; opt++)
		if (opt->tmp != NULL)
			opt->setter(ctx, ((char *) ctx) + opt->ofs, opt->tmp);

#ifdef HAVE_SSL
	/* If SSL is wanted and port is not overridden, set it to 443 */
	if (ctx->port == atoi(PORT) && ctx->ssl_ctx != NULL)
		ctx->port = 443;
#endif
	/* If document_root is not set, set it to current directory */
	if (ctx->root[0] == '\0')
		(void) getcwd(ctx->root, sizeof(ctx->root));

	debug("do_init: initialized context %p", ctx);

	return (ctx);
}

/*
 * Deallocate shttpd object, free up the resources
 */
void
shttpd_fini(struct shttpd_ctx *ctx)
{
	struct mimetype	*p, *tmp;
	struct conn	*c, *nc;
	struct userurl *u, *u1;

	/* TODO: Free configuration */

	/* Free allocated mime-types */
	for (p = ctx->mimetypes; p != NULL; p = tmp) {
		tmp = p->next;
		free(p->ext);
		free(p->mime);
		free(p);
	}

	/* Free all connections */
	for (c = ctx->connections; c != NULL; c = nc) {
		nc = c->next;
		c->flags = 0;
		disconnect(c);
	}

	/* Free allocated userurls */
	for (u = ctx->urls; u != NULL; u = u1) {
		u1 = u->next;
		free(u->url);
		free(u);
	}

	if (ctx->index) {
		free(ctx->index);
		ctx->index = NULL;
	}
	if (ctx->ext) {
		free(ctx->ext);
		ctx->ext = NULL;
	}
	if (ctx->interp) {
		free(ctx->interp);
		ctx->interp = NULL;
	}
	if (ctx->realm) {
		free(ctx->realm);
		ctx->realm = NULL;
	}

	if (ctx->pass) {
		free(ctx->pass);
		ctx->pass = NULL;
	}
	if (ctx->put_auth) {
		free(ctx->put_auth);
		ctx->put_auth = NULL;
	}
	if (ctx->uid) {
		free(ctx->uid);
		ctx->uid = NULL;
	}

	if (ctx->accesslog)	(void) fclose(ctx->accesslog);
	free(ctx);
}



