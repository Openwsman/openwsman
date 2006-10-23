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
#endif

#define	SHTTPD_VERSION		"1.35"		/* Version			*/
#ifndef CONFIG
#define	CONFIG		"/usr/local/etc/shttpd.conf"	/* Configuration file		*/
#endif /* CONFIG */
#define	HTPASSWD	".htpasswd"	/* Passwords file name		*/
#define	EXPIRE_TIME	3600		/* Expiration time, seconds	*/
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

#ifdef _WIN32		/* Windows specific #includes and #defines */
#pragma comment(lib,"ws2_32")
#pragma comment(lib,"user32")
#pragma comment(lib,"comctl32")
#pragma comment(lib,"comdlg32")
#pragma comment(lib,"shell32")
#pragma comment(linker,"/subsystem:console")
#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <direct.h>
#include <io.h>
#include <shlobj.h>
#define	ERRNO			GetLastError()
#define	NO_SOCKLEN_T
#define	SSL_LIB			"libssl32.dll"
typedef unsigned int		uint32_t;
typedef unsigned short		uint16_t;
#define	S_ISDIR(x)		((x) & _S_IFDIR)
#define	DIRSEP			'\\'
#define	O_NONBLOCK		0
#define	waitpid(a,b,c)		0
#define	EWOULDBLOCK		WSAEWOULDBLOCK
#define	snprintf		_snprintf
#define	vsnprintf		_vsnprintf
#define	mkdir(x,y)		_mkdir(x)
#define	dlopen(x,y)		LoadLibrary(x)
#define	dlsym(x,y)		(void *) GetProcAddress(x,y)
#define	_POSIX_
static char		**Argv;		/* argv passed to main() */
static int		Argc;		/* argc passed to main() */
static HICON		hIcon;		/* SHTTPD icon handle */

/* POSIX dirent interface */
struct dirent {
	char	*d_name;
};
typedef struct DIR {
	long			handle;
	struct _finddata_t	info;
	struct dirent		result;
	char			*name;
} DIR;

#else			/* UNIX specific #includes and #defines */

#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>		/* Some linuxes put struct timeval there */

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
#endif	/* _WIN32 */

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

#ifdef EMBEDDED
#include "shttpd.h"
#endif /* EMBEDDED */

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

typedef struct ssl_st SSL;
typedef struct ssl_method_st SSL_METHOD;
typedef struct ssl_ctx_st SSL_CTX;

#define	SSL_ERROR_WANT_READ	2
#define	SSL_ERROR_WANT_WRITE	3
#define SSL_FILETYPE_PEM	1

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
	char	buf[IO_MAX];		/* Buffer		*/
	int	done;			/* IO finished		*/
	size_t	head;			/* Bytes read		*/
	size_t	tail;			/* Bytes written	*/
};
#define	IO_SPACELEN(io)		(sizeof((io)->buf) - (io)->head - 1)
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
	SSL		*ssl;		/* SSL descriptor		*/
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
	char		ouri[IO_MAX];	/* Original unmodified URI	*/
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
#define	FLAG_FINISHED		1	/* Connection to be closed	*/
#define	FLAG_PARSED		2	/* Request has been parsed	*/
#define	FLAG_CGIPARSED		4	/* CGI output has been parsed	*/
#define	FLAG_SSLACCEPTED	8	/* SSL_accept() succeeded	*/
#define	FLAG_ALWAYS_READY	16	/* Local channel always ready for IO */
#define	FLAG_USER_WATCH		32	/* User watch			*/
#define	FLAG_SOCK_READABLE	64
#define	FLAG_SOCK_WRITABLE	128
#define FLAG_FD_READABLE	256
#define	FLAG_FD_WRITABLE	512
#define	FLAG_CGI		1024
#define	FLAG_KEEP_CONNECTION	2048
};

#define	FLAG_IO_READY	(FLAG_SOCK_WRITABLE | FLAG_SOCK_READABLE | \
		FLAG_FD_READABLE | FLAG_FD_WRITABLE)

enum err_level	{ERR_DEBUG, ERR_INFO, ERR_FATAL};
#ifdef OPENWSMAN
#include "u/libu.h"

#define elog(level, ...) \
    do { \
        debug( __VA_ARGS__ ); \
        if (level == ERR_FATAL) exit(EXIT_FAILURE); \
    } while (0)
#endif

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
 * Dynamically loaded SSL functionality
 */
static struct ssl_func {
	const char	*name;			/* SSL function name */
	union variant	ptr;			/* Function pointer */
} ssl_sw[] = {
	{"SSL_free",			{0}},
	{"SSL_accept",			{0}},
	{"SSL_connect",			{0}},
	{"SSL_read",			{0}},
	{"SSL_write",			{0}},
	{"SSL_get_error",		{0}},
	{"SSL_set_fd",			{0}},
	{"SSL_new",			{0}},
	{"SSL_CTX_new",			{0}},
	{"SSLv23_server_method",	{0}},
	{"SSL_library_init",		{0}},
	{"SSL_CTX_use_PrivateKey_file",	{0}},
	{"SSL_CTX_use_certificate_file",{0}},
    {"SSL_pending",{0}},
	{NULL,				{0}}
};
#define	FUNC(x)	ssl_sw[x].ptr.value_func
#define	SSL_free(x)	(* (void (*)(SSL *)) FUNC(0))(x)
#define	SSL_accept(x)	(* (int (*)(SSL *)) FUNC(1))(x)
#define	SSL_connect(x)	(* (int (*)(SSL *)) FUNC(2))(x)
#define	SSL_read(x,y,z)	(* (int (*)(SSL *, void *, int)) FUNC(3))((x),(y),(z))
#define	SSL_write(x,y,z) \
	(* (int (*)(SSL *, const void *,int)) FUNC(4))((x), (y), (z))
#define	SSL_get_error(x,y)(* (int (*)(SSL *, int)) FUNC(5))((x), (y))
#define	SSL_set_fd(x,y)	(* (int (*)(SSL *, int)) FUNC(6))((x), (y))
#define	SSL_new(x)	(* (SSL * (*)(SSL_CTX *)) FUNC(7))(x)
#define	SSL_CTX_new(x)	(* (SSL_CTX * (*)(SSL_METHOD *)) FUNC(8))(x)
#define	SSLv23_server_method()	(* (SSL_METHOD * (*)(void)) FUNC(9))()
#define	SSL_library_init() (* (int (*)(void)) FUNC(10))()
#define	SSL_CTX_use_PrivateKey_file(x,y,z)	(* (int (*)(SSL_CTX *, \
		const char *, int)) FUNC(11))((x), (y), (z))
#define	SSL_CTX_use_certificate_file(x,y,z)	(* (int (*)(SSL_CTX *, \
		const char *, int)) FUNC(12))((x), (y), (z))
#define SSL_pending(x) (* (int (*)(SSL *)) FUNC(13))(x)

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
#ifdef EMBEDDED
	shttpd_callback_t	func;
#endif /* EMBEDDED */
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
	SSL_CTX		*ssl_ctx;		/* SSL context */
#ifdef _WIN32
	CRITICAL_SECTION mutex;			/* For MT case */
	HANDLE		ev[2];			/* For thread synchronization */
#endif /* _WIN32*/

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
#ifdef OPENWSMAN
        shttpd_bauth_callback_t         bauthf; /* basic authorization callback */
        shttpd_dauth_callback_t         dauthf; /* digest authorization callback */
#endif
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
#ifndef EMBEDDED
static const char	*config_file;	/* Configuration file */
#endif /* EMBEDDED */

/*
 * Prototypes
 */
static void	io_inc_tail(struct io *io, size_t n);
#ifndef OPENWSMAN
static void	elog(enum err_level, const char *fmt, ...);
#endif
static int	casecmp(register const char *s1, register const char *s2);
static int	ncasecmp(register const char *, register const char *, size_t);
static char	*mystrdup(const char *str);
#ifndef OPENWSMAN
static int	Open(const char *path, int flags, int mode);
static int	Stat(const char *path, struct stat *stp);
#endif
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
#ifndef OPENWSMAN
static void	get_dir(struct conn *c);
static void	do_dir(struct conn *c);
static void	get_file(struct conn *c);
static void	do_get(struct conn *c);
#endif
static void	urldecode(char *from, char *to);
static void	killdots(char *file);
static char	*fetch(const char *src, char *dst, size_t len);
static void     parse_headers(struct conn *c, char *s);
static void	parse_request(struct conn *c);
#ifndef OPENWSMAN
static int	useindex(struct conn *, char *path, size_t maxpath);
#endif
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

static void
set_ssl(struct shttpd_ctx *ctx, void *arg, const char *pem)
{
	SSL_CTX		*CTX;
	void		*lib;
	struct ssl_func	*fp;

	arg = NULL;	/* Unused */
	/* Load SSL library dynamically */
	if ((lib = dlopen(SSL_LIB, RTLD_LAZY)) == NULL) {
		elog(ERR_FATAL, "set_ssl: cannot load %s", SSL_LIB);
    }

	for (fp = ssl_sw; fp->name != NULL; fp++)
		if ((fp->ptr.value_void = dlsym(lib, fp->name)) == NULL)
			elog(ERR_FATAL, "set_ssl: cannot find %s", fp->name);

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
	{'s', "ssl_certificate", "SSL certificate file", set_ssl,
		OFS(ssl_ctx), "pem_file", NULL, NULL, OPT_FLAG_FILE},
    {'k', "ssl_priv_key", "SSL pivate key file", set_ssl_priv_key,
        OFS(ssl_ctx), "pem_file", NULL, NULL, OPT_FLAG_FILE},
	{'U', "put_auth", "PUT,DELETE auth file",set_str,
		OFS(put_auth), "file", NULL, NULL, OPT_FLAG_FILE},
	{'V', "cgi_envvar", "CGI envir variables", set_envvars,
		OFS(envvars), "X=Y,....", NULL, NULL, 0		},
	{'a', "aliases", "Aliases", set_aliases,
		OFS(mountpoints), "X=Y,...", NULL, NULL, 0	},
#ifdef _WIN32
	{'g', "enable_gui", "GUI support", set_int,
		OFS(gui), BOOL_OPT, "1", NULL, OPT_FLAG_BOOL	},
#else
	{'I', "inetd_mode", "Inetd mode", set_inetd,
		0, BOOL_OPT, "0", NULL, OPT_FLAG_BOOL	},
	{'u', "runtime_uid", "Run as user", set_str,
		OFS(uid), "user_name", NULL, NULL, 0		},
#endif /* _WIN32 */
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

static struct mountpoint *
ismountpoint(struct shttpd_ctx *ctx, const char *url)
{
	struct mountpoint	*p;

	for (p = ctx->mountpoints; p != NULL; p = p->next)
		if (strncmp(url, p->mountpoint, strlen(p->mountpoint)) == 0)
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
                        elog(ERR_DEBUG,"do_embedded: c->nposted = %d; cclength = %d", c->nposted, c->cclength);
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
	assert(c->local.head <= sizeof(c->local.buf));

	if (arg.last) {
		c->local.done++;
		c->io = NULL;
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
		const char *url, shttpd_callback_t cb, void *data)
{
	struct userurl	*p;

	if ((p = calloc(1, sizeof(*p))) != NULL) {
		p->func		= cb;
		p->data		= data;
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

	
int
shttpd_get_post_query(struct shttpd_arg_t *arg, char *to, int length)
{
	struct conn	*c = arg->priv;
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
	assert(io->tail <= io->head);
	assert(io->head <= sizeof(io->buf));
	io->tail += n;
	assert(io->tail <= io->head);
	if (io->tail == io->head)
		io->head = io->tail = 0;
}

#ifdef _WIN32
static void
fix_directory_separators(char *path)
{
	for (; *path != '\0'; path++) {
		if (*path == '/')
			*path = '\\';
		if (*path == '\\')
			while (path[1] == '\\' || path[1] == '/') 
				(void) strcpy(path + 1, path + 2);
	}
}
#endif	/* _WIN32 */


#ifndef OPENWSMAN
/*
 * Wrapper around open(), that takes care about directory separators
 */
static int
Open(const char *path, int flags, int mode)
{
	int	fd;

#ifdef _WIN32
	char	buf[FILENAME_MAX];

	mystrlcpy(buf, path, sizeof(buf));
	fix_directory_separators(buf);
	fd = _open(buf, flags);
#else
	if ((fd = open(path, flags, mode)) != -1)
		(void) fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif /* _WIN32 */

	return (fd);
}
#endif


#ifndef OPENWSMAN

/*
 * The wrapper around stat(), that takes care about directory separators
 */
static int
Stat(const char *path, struct stat *stp)
{
#ifdef _WIN32
	char	buf[FILENAME_MAX], *p;

	mystrlcpy(buf, path, sizeof(buf));
	fix_directory_separators(buf);
	p = buf + strlen(buf) - 1;
	while (p > buf && *p == '\\' && p[-1] != ':')
		*p-- = '\0';
	path = buf;
#endif /* _WIN32  */

	return (stat(path, stp));
}
#endif //OPENWSMAN
#if defined(_WIN32) && !defined(__GNUC__)
/*
 * POSIX directory management (limited implementation, enough for shttpd)
 */
static DIR *
opendir(const char *name)
{
	DIR		*dir = NULL;
	size_t		base_length;
	const char	*all;

	if (name && name[0]) {
		base_length = strlen(name);
		all = strchr("/\\", name[base_length - 1]) ? "*" : "/*";

		if ((dir = malloc(sizeof *dir)) != NULL &&
		    (dir->name = malloc(base_length + strlen(all) + 1)) != 0) {
			(void) strcat(strcpy(dir->name, name), all);

			if ((dir->handle = (long) _findfirst(dir->name,
			    &dir->info)) != -1) {
				dir->result.d_name = 0;
			} else {
				free(dir->name);
				free(dir);
				dir = 0;
			}
		} else {
			free(dir);
			dir = NULL;
			errno = ENOMEM;
		}
	} else {
		errno = EINVAL;
	}

	return (dir);
}

static int
closedir(DIR *dir)
{
	int result = -1;

	if (dir) {
		if(dir->handle != -1)
			result = _findclose(dir->handle);

		free(dir->name);
		free(dir);
	}

	if (result == -1) 
		errno = EBADF;

	return (result);
}

static struct dirent *
readdir(DIR *dir)
{
	struct dirent *result = 0;

	if (dir && dir->handle != -1) {
		if(!dir->result.d_name ||
		    _findnext(dir->handle, &dir->info) != -1) {
			result = &dir->result;
			result->d_name = dir->info.name;
		}
	} else {
		errno = EBADF;
	}

	return (result);
}
#endif /* _WIN32 */

#ifndef OPENWSMAN

/*
 * Log function
 */
static void
elog(enum err_level level, const char *fmt, ...)
{
	va_list		ap;

	if (inetd || (_debug == 0 && level == ERR_DEBUG))
		return;

	(void) fprintf(stderr, "%lu ", (unsigned long) current_time);
	
	va_start(ap, fmt);
	(void) vfprintf(stderr, fmt, ap);
	(void) fputc('\n', stderr);
	fflush(stderr);
	va_end(ap);

#ifdef _WIN32
        if (level == ERR_FATAL) {
		char	msg[512];
		va_start(ap, fmt);
		vsnprintf(msg, sizeof(msg), fmt, ap);
		va_end(ap);
		MessageBox(NULL, msg, "Error", MB_OK | MB_ICONEXCLAMATION);
	}
#endif /* _WIN32 */   

	if (level == ERR_FATAL)
		exit(EXIT_FAILURE);
}

#endif

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
    if (c->query) {
        free(c->query);
        c->query = NULL;
    }
    if (c->range) {
        free(c->range);
        c->range = NULL;
    }

    if (c->query) {
        free(c->query);
        c->query = NULL;
    }
    c->cclength = 0;
}



static void
disconnect(struct conn *c)
{

	elog(ERR_DEBUG, "disconnecting %p", c);

	del_conn_from_ctx(c->ctx, c);

	if (c->ctx->accesslog != NULL)
		log_access(c->ctx->accesslog, c);

	/* In inetd mode, exit if request is finished. */
	if (inetd)
		exit_flag++;

	free_parsed_data(c);

    if (c->usr) {
        free(c->usr);
        c->usr = NULL;
    }
    if (c->pwd) {
        free(c->pwd);
        c->pwd = NULL;
    }


	/* Free resources */
	if (c->fd != -1) {
		if (c->flags & FLAG_CGI)
			(void) closesocket(c->fd);
		else
			(void) close(c->fd);
	}
	if (c->dirp)		(void) closedir(c->dirp);
	if (c->ssl)		SSL_free(c->ssl);

	(void) shutdown(c->sock, 2);
	(void) closesocket(c->sock);
	free(c);
}

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
	if (c->ssl)
		n = SSL_write(c->ssl, buf, len);
	else
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
	} else {
		n = recv(c->sock, buf, len, 0);
    }
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
#ifdef	_WIN32
	unsigned long	on = 1;

	return (ioctlsocket(fd, FIONBIO, &on));
#else
	int	flags;

	if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
		elog(ERR_INFO, "nonblock: fcntl(F_GETFL): %d", ERRNO);
	else if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0)
		elog(ERR_INFO, "nonblock: fcntl(F_SETFL): %d", ERRNO);
	else
		ret = 0;	/* Success */

	return (ret);
#endif /* _WIN32 */
}

/*
 * Setup listening socket on given port, return socket
 */
int
shttpd_open_port(int port)
{
	int		sock, on = 1;
	struct usa	sa;

#ifdef _WIN32
	{WSADATA data;	WSAStartup(MAKEWORD(2,2), &data);}
#endif /* _WIN32 */

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
#ifndef _WIN32
	(void) fcntl(sock, F_SETFD, FD_CLOEXEC);
#endif /* !_WIN32 */

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
	    date, c->method, c->ouri, c->proto, c->status,
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
	char	msg[sizeof(c->local.buf)];
	int	n;

	c->shlength = n = Snprintf(msg, sizeof(msg),
	   "HTTP/1.1 %d %s\r\nConnection: close\r\n%s%s\r\n%d ",
	    status, descr, headers, headers[0] == '\0' ? "" : "\r\n", status);
	va_start(ap, fmt);
	n += vsnprintf(msg + n, sizeof(msg) - n, fmt, ap);
	if (n > (int) sizeof(msg))
		n = sizeof(msg);
	va_end(ap);
	mystrlcpy(c->local.buf, msg, sizeof(c->local.buf));
	c->local.head = n;
	c->local.tail = 0;
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


#ifndef OPENWSMAN
/*
 * For a given PUT path, create all intermediate subdirectories
 * for given path. Return 0 if the path itself is a directory,
 * or -1 on error, 1 if OK.
 */
static int
put_dir(const char *path)
{
	char		buf[FILENAME_MAX];
	const char	*s, *p;
	struct stat	st;
	size_t		len;

	for (s = p = path + 2; (p = strchr(s, '/')) != NULL; s = ++p) {
		len = p - path;
		assert(len < sizeof(buf));
		(void) memcpy(buf, path, len);
		buf[len] = '\0';

		/* Try to create intermediate directory */
		if (Stat(buf, &st) == -1 && mkdir(buf, 0755) != 0)
			return (-1);

		/* Is path itself a directory ? */
		if (p[1] == '\0')
			return (0);
	}

	return (1);
}

/*
 * PUT request
 */
static void
put_file(struct conn *c)
{
	int	n, len;
	void	*buf = c->remote.buf + c->remote.tail;

	assert(c->fd != -1);
	if ((len = IO_DATALEN(&c->remote)) <= 0)
		return;

	n = write(c->fd, buf, len);
	elog(ERR_DEBUG, "put_file(%p, %d): %d bytes", c, len, n);

	if (n > 0) {
		io_inc_tail(&c->remote, n);
		c->nposted += n;
	}
	
	if (n <= 0 || c->nposted >= c->cclength) {
		(void) fstat(c->fd, &c->st);
		c->local.head = c->shlength = Snprintf(c->local.buf,
		    sizeof(c->local.buf),
		    "HTTP/1.1 %d OK\r\n"
		    "Content-Length: %u\r\n"
		    "Connection: close\r\n\r\n", c->status, c->st.st_size);
		c->local.done++;
		c->io = NULL;
	}
}

/*
 * GET the directory
 */
static void
get_dir(struct conn *c)
{
	struct dirent	*dp = NULL;
	char		file[FILENAME_MAX], line[FILENAME_MAX + 512],
				size[64], mod[64];
	const char	*slash;
	struct stat	st;
	int		n, left;

	assert(c->dirp != NULL);
	assert(c->uri[0] != '\0');

	left = IO_SPACELEN(&c->local);
	slash = c->uri[strlen(c->uri) - 1] == '/' ? "" : "/";

	do {
		if (left < (int) sizeof(line))
			break;

		if ((dp = readdir(c->dirp)) == NULL) {
			/* Finished reading directory */
			c->local.done++;
			c->io = NULL;
			break;
		}

		/* Do not show current dir and passwords file */
		if (strcmp(dp->d_name, ".") == 0 ||
		   strcmp(dp->d_name, HTPASSWD) == 0)
			continue;

		(void) snprintf(file, sizeof(file),
		    "%s%s%s",c->path, slash, dp->d_name);
		(void) Stat(file, &st);
		if (S_ISDIR(st.st_mode)) {
			snprintf(size,sizeof(size),"%s","&lt;DIR&gt;");
		} else {
			if (st.st_size < 1024)
				(void) snprintf(size, sizeof(size),
				    "%lu", (unsigned long) st.st_size);
			else if (st.st_size < 1024 * 1024)
				(void) snprintf(size, sizeof(size), "%luk",
				    (unsigned long) (st.st_size >> 10)  + 1);
			else
				(void) snprintf(size, sizeof(size),
				    "%.1fM", (float) st.st_size / 1048576);
		}
		(void) strftime(mod, sizeof(mod),
		    "%d-%b-%Y %H:%M", localtime(&st.st_mtime));

		n = Snprintf(line, sizeof(line),
		    "<tr><td><a href=\"%s%s%s\">%s%s</a></td>"
		    "<td>%s</td><td>&nbsp;&nbsp;%s</td></tr>\n",
		    c->uri, slash, dp->d_name, dp->d_name,
		    S_ISDIR(st.st_mode) ? "/" : "", mod, size);
		(void) memcpy(c->local.buf + c->local.head, line, n);
		c->local.head += n;
		left -= n;
	} while (dp != NULL);
}


/*
 * Schedule GET for the directory
 */
static void
do_dir(struct conn *c)
{
	if ((c->dirp = opendir(c->path)) == NULL) {
		senderr(c, 500, "Error","", "Cannot open dir");
	} else {
		c->local.head = Snprintf(c->local.buf, sizeof(c->local.buf),
		    "HTTP/1.1 200 OK\r\n"
		    "Content-Type: text/html\r\n"
		    "\r\n"
		    "<html><head><title>Index of %s</title>"
		    "<style>th {text-align: left;}</style></head>"
		    "<body><h1>Index of %s</h1><pre><table cellpadding=\"0\">"
		    "<tr><th>Name</th><th>Modified</th><th>Size</th></tr>"
		    "<tr><td colspan=\"3\"><hr></td></tr>",
		    c->uri, c->uri);
		c->io = get_dir;
		c->remote.head = 0;
		c->status = 200;
		c->flags |= FLAG_ALWAYS_READY;
	}
}

/*
 * GET regular file
 */
static void
get_file(struct conn *c)
{
	int	n;

	assert(c->fd != -1);
	n = read(c->fd, c->local.buf + c->local.head, IO_SPACELEN(&c->local));

	if (n > 0) {
		c->local.head += n;
	} else {
		c->local.done++;
		c->io = NULL;
	}
}

/*
 * Schedule GET for regular file
 */
static void
do_get(struct conn *c)
{
	char		date[64], lm[64], etag[64], range[64] = "";
	int		n, status = 200;
	unsigned long	r1, r2;
	const char	*mime = "text/plain", *msg = "OK";
	const char	*fmt, *s = c->uri + strlen(c->uri);
	struct mimetype	*p;

	/* Figure out the mime type */
	for (p = c->ctx->mimetypes; p != NULL; p = p->next)
		if (strlen(c->uri) > p->extlen &&
		    *(s - p->extlen - 1) == '.' &&
		    !ncasecmp(p->ext, s - p->extlen, p->extlen)) {
			mime = p->mime;
			break;
		}

	c->sclength = (unsigned long) c->st.st_size;

	/* If Range: header specified, act accordingly */
	if (c->range && (n = sscanf(c->range, "bytes=%lu-%lu", &r1, &r2)) > 0) {
		status = 206;
		(void) lseek(c->fd, r1, SEEK_SET);
		c->sclength = n == 2 ? r2 - r1 + 1: c->sclength - r1;
		(void) Snprintf(range, sizeof(range),
		    "Content-Range: bytes %lu-%lu/%lu\r\n",
		    r1, r1 + c->sclength, (unsigned long) c->st.st_size);
		msg = "Partial Content";
	}

	/* Prepare Etag, Date, Last-Modified headers */
	fmt = "%a, %d %b %Y %H:%M:%S GMT";
	(void) strftime(date, sizeof(date), fmt, localtime(&current_time));
	(void) strftime(lm, sizeof(lm), fmt, localtime(&c->st.st_mtime));
	(void) snprintf(etag, sizeof(etag), "%lx.%lx",
	    (unsigned long) c->st.st_mtime, (unsigned long) c->st.st_size);

	/* Local read buffer should be empty */
	c->local.head = c->shlength = Snprintf(c->local.buf,
	    sizeof(c->local.buf),
	    "HTTP/1.1 %d %s\r\n"
	    "Date: %s\r\n"
	    "Last-Modified: %s\r\n"
	    "Etag: \"%s\"\r\n"
	    "Content-Type: %s\r\n"
	    "Content-Length: %lu\r\n"
	    "Connection: close\r\n"
	    "%s\r\n",
	    status, msg, date, lm, etag, mime, c->sclength, range);
	c->status = status;
	elog(ERR_DEBUG, "get_file: [%s]", c->local.buf);

	c->remote.head = 0;
	c->flags |= FLAG_ALWAYS_READY;
	if (c->http_method == METHOD_GET)
		c->io = get_file;
	else if (c->http_method == METHOD_HEAD)
		c->local.done++;
}

#endif

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


#ifndef OPENWSMAN

/*
 * I have snatched MD5 code from the links browser project,
 * and including the copyright message here along with the code. (Sergey)
 */

/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 */

/*
 * MD5 crypto algorithm.
 */
typedef struct _MD5Context {
	uint32_t	buf[4];
	uint32_t	bits[2];
	unsigned char	in[64];
} _MD5_CTX;

#if __BYTE_ORDER == 1234
#define byteReverse(buf, len)	/* Nothing */
#else
/*
 * Note: this code is harmless on little-endian machines.
 */
static void byteReverse(unsigned char *buf, unsigned longs)
{
	uint32_t t;
	do {
		t = (uint32_t) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
			((unsigned) buf[1] << 8 | buf[0]);
		*(uint32_t *) buf = t;
		buf += 4;
	} while (--longs);
}
#endif /* __BYTE_ORDER */

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
static void _MD5Init(_MD5_CTX *ctx)
{
	ctx->buf[0] = 0x67452301;
	ctx->buf[1] = 0xefcdab89;
	ctx->buf[2] = 0x98badcfe;
	ctx->buf[3] = 0x10325476;

	ctx->bits[0] = 0;
	ctx->bits[1] = 0;
}

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
static void _MD5Transform(uint32_t buf[4], uint32_t const in[16])
{
	register uint32_t a, b, c, d;

	a = buf[0];
	b = buf[1];
	c = buf[2];
	d = buf[3];

	MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
	MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
	MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
	MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
	MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
	MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
	MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
	MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
	MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
	MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
	MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
	MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
	MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
	MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
	MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
	MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

	MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
	MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
	MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
	MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
	MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
	MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
	MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
	MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
	MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
	MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
	MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
	MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
	MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
	MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
	MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
	MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

	MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
	MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
	MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
	MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
	MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
	MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
	MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
	MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
	MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
	MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
	MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
	MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
	MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
	MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
	MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
	MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

	MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
	MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
	MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
	MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
	MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
	MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
	MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
	MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
	MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
	MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
	MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
	MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
	MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
	MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
	MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
	MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
static void
_MD5Update(_MD5_CTX *ctx, unsigned char const *buf, unsigned len)
{
	uint32_t t;

	/* Update bitcount */

	t = ctx->bits[0];
	if ((ctx->bits[0] = t + ((uint32_t) len << 3)) < t)
		ctx->bits[1]++;		/* Carry from low to high */
	ctx->bits[1] += len >> 29;

	t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

	/* Handle any leading odd-sized chunks */

	if (t) {
		unsigned char *p = (unsigned char *) ctx->in + t;

		t = 64 - t;
		if (len < t) {
			memcpy(p, buf, len);
			return;
		}
		memcpy(p, buf, t);
		byteReverse(ctx->in, 16);
		_MD5Transform(ctx->buf, (uint32_t *) ctx->in);
		buf += t;
		len -= t;
	}
	/* Process data in 64-byte chunks */

	while (len >= 64) {
		memcpy(ctx->in, buf, 64);
		byteReverse(ctx->in, 16);
		_MD5Transform(ctx->buf, (uint32_t *) ctx->in);
		buf += 64;
		len -= 64;
	}

	/* Handle any remaining bytes of data. */

	memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern 
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
static void
_MD5Final(unsigned char digest[16], _MD5_CTX *ctx)
{
	unsigned count;
	unsigned char *p;

	/* Compute number of bytes mod 64 */
	count = (ctx->bits[0] >> 3) & 0x3F;

	/* Set the first char of padding to 0x80.  This is safe since there is
	   always at least one byte free */
	p = ctx->in + count;
	*p++ = 0x80;

	/* Bytes of padding needed to make 64 bytes */
	count = 64 - 1 - count;

	/* Pad out to 56 mod 64 */
	if (count < 8) {
		/* Two lots of padding:  Pad the first block to 64 bytes */
		memset(p, 0, count);
		byteReverse(ctx->in, 16);
		_MD5Transform(ctx->buf, (uint32_t *) ctx->in);

		/* Now fill the next block with 56 bytes */
		memset(ctx->in, 0, 56);
	} else {
		/* Pad block to 56 bytes */
		memset(p, 0, count - 8);
	}
	byteReverse(ctx->in, 14);

	/* Append length in bits and transform */
	((uint32_t *) ctx->in)[14] = ctx->bits[0];
	((uint32_t *) ctx->in)[15] = ctx->bits[1];

	_MD5Transform(ctx->buf, (uint32_t *) ctx->in);
	byteReverse((unsigned char *) ctx->buf, 4);
	memcpy(digest, ctx->buf, 16);
	memset((char *) ctx, 0, sizeof(ctx));	/* In case it's sensitive */
}

/*
 * END OF LICENSED MD5 CODE (Sergey)
 */

/*
 * Stringify binary data. Output buffer must be twice as big as input,
 * because each byte takes 2 bytes in string representation
 */
static void
bin2str(char *to, const unsigned char *p, size_t len)
{
	const char	*hex = "0123456789abcdef";

	for (;len--; p++) {
		*to++ = hex[p[0] >> 4];
		*to++ = hex[p[0] & 0x0f];
	}
	*to = '\0';
}

/*
 * Return stringified MD5 hash for list of vectors.
 * buf must point to at least 32-bytes long buffer
 */
static void
md5(char *buf, ...)
{
	unsigned char		hash[16];
	const unsigned char	*p;
	va_list			ap;
	_MD5_CTX		ctx;

	_MD5Init(&ctx);

	va_start(ap, buf);
	while ((p = va_arg(ap, const unsigned char *)) != NULL)
		_MD5Update(&ctx, p, strlen((char *) p));
	va_end(ap);

	_MD5Final(hash, &ctx);
	bin2str(buf, hash, sizeof(hash));
}

#endif

#ifndef EMBEDDED
/*
 * Edit the passwords file.
 */
static int
editpass(const char *fname, const char *domain,
		const char *user, const char *pass)
{

#define	LSIZ		512
	int		ret = EXIT_SUCCESS, found = 0;
	char		line[LSIZ], tmp[FILENAME_MAX],
				u[LSIZ], d[LSIZ], ha1[LSIZ];
	FILE		*fp = NULL, *fp2 = NULL;

	(void) snprintf(tmp, sizeof(tmp), "%s.tmp", fname);

	/* Create the file if does not exist */
	if ((fp = fopen(fname, "a+")))
		(void) fclose(fp);

	/* Open the given file and temporary file */
	if ((fp = fopen(fname, "r")) == NULL)
		elog(ERR_FATAL, "Cannot open %s: %s", fname, strerror(errno));
	else if ((fp2 = fopen(tmp, "w+")) == NULL)
		elog(ERR_FATAL, "Cannot open %s: %s", tmp, strerror(errno));

	/* Copy the stuff to temporary file */
	while (fgets(line, sizeof(line), fp) != NULL) {
		if (sscanf(line, "%[^:]:%[^:]:%s", u, d, ha1) == 3 &&
		    strcmp(user, u) == 0 &&
		    strcmp(domain, d) == 0) {
			found++;
			md5(ha1, user, ":", domain, ":", pass, NULL);
			(void) snprintf(line, sizeof(line),
			    "%s:%s:%s\n", user, domain, ha1);
		}
		(void) fprintf(fp2, "%s", line);
	}

	/* If new user, just add it */
	if (found == 0) {
		md5(ha1, user, ":", domain, ":", pass, NULL);
		(void) snprintf(line, sizeof(line),
		    "%s:%s:%s\n", user, domain, ha1);
		(void) fprintf(fp2, "%s", line);
	}

	/* Close files */
	(void) fclose(fp);
	(void) fclose(fp2);

	/* Put the temp file in place of real file */
	(void) remove(fname);
	(void) rename(tmp, fname);

	return (ret);
}
#endif /* !EMBEDDED */



/*
 * HTTP digest authentication
 */
#ifndef OPENWSMAN
struct digest {
	char		user[USER_MAX];
	char		uri[IO_MAX];
	char		cnonce[64];
	char		nonce[33];
	char		resp[33];
	char		qop[16];
	char		nc[16];
};

#else

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

#endif /* OPENWSMAN */

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
		else if (ncasecmp(p, "cnonce=", 7) == 0)
			fetchfield(&p, dig->cnonce, sizeof(dig->cnonce), 7);
		else if (ncasecmp(p, "nc=", 3) == 0)
			fetchfield(&p, dig->nc, sizeof(dig->cnonce), 3);

	elog(ERR_DEBUG, "[%s] [%s] [%s] [%s] [%s] [%s]",
	    dig->user, dig->uri, dig->resp, dig->qop, dig->cnonce, dig->nc);

	return (1);
}

#ifndef OPENWSMAN
/*
 * Check the user's password, return 1 if OK
 */
static int
checkpass(const struct conn *c, const char *a1, const struct digest *dig)
{
	char		a2[33], resp[33];

	/* XXX  Due to a bug in MSIE, we do not compare the URI	 */
	/* Also, we do not check for authentication timeout */
	if (/*strcmp(dig->uri, c->ouri) != 0 || */
	    strlen(dig->resp) != 32 /*||
	    now - strtoul(dig->nonce, NULL, 10) > 3600 */)
		return (0);

	md5(a2, c->method, ":", dig->uri, NULL);
	md5(resp, a1, ":", dig->nonce, ":", dig->nc, ":",
	    dig->cnonce, ":", dig->qop, ":", a2, NULL);
	elog(ERR_DEBUG, "checkpass: [%s] [%s]", resp, dig->resp);

	return (strcmp(resp, dig->resp) == 0);
}

static FILE *
open_auth_file(struct shttpd_ctx *ctx, const char *path)
{
	char 		name[FILENAME_MAX];
	const char	*p, *e;
	FILE		*fp = NULL;
	int		fd;

	if (ctx->pass) {
		/* Use global passwords file */
		(void) snprintf(name, sizeof(name), "%s", ctx->pass);
	} else {
		/* Try to find .htpasswd in requested directory */
		for (p = path, e = p + strlen(p) - 1; e > p; e--)
			if (*e == '/')
				break;

		assert(*e == '/');
		(void) Snprintf(name, sizeof(name), "%.*s/%s",
		    (int) (e - p), p, HTPASSWD);
	}

	if ((fd = Open(name, O_RDONLY, 0)) == -1) {
		elog(ERR_DEBUG, "open_auth_file: open(%s)", name);
	} else if ((fp = fdopen(fd, "r")) == NULL) {
		elog(ERR_DEBUG, "open_auth_file: fdopen(%s)", name);
		(void) close(fd);
	}

	return (fp);
}

/*
 * Authorize against the opened passwords file
 */
static int
authorize(struct conn *c, FILE *fp)
{
	int		authorized = 0;
	char		l[256], u[65], ha1[65], dom[65];
	struct digest	di;

	if (c->auth && getauth(c, &di) && (c->user = mystrdup(di.user)) != NULL)
		while (fgets(l, sizeof(l), fp) != NULL) {
			if (sscanf(l, "%64[^:]:%64[^:]:%64s", u, dom, ha1) != 3)
				continue;	/* Ignore malformed lines */
			if (!strcmp(c->user, u) && !strcmp(c->ctx->realm,dom)) {
				authorized = checkpass(c, ha1, &di);
				break;
			}
		}

	return (authorized);
}



/*
 * Check authorization. Return 1 if not needed or authorized, 0 otherwise
 */
static int
checkauth(struct conn *c, const char *path)
{
	FILE	*fp = NULL;
	int	authorized = 1;
	
#ifdef EMBEDDED
	struct userauth	*uap;

	/* Check, is this URL protected by shttpd_protect_url() */
	for (uap = c->ctx->auths; uap != NULL; uap = uap->next) {
		if (strncmp(uap->url, c->uri, strlen(uap->url)) == 0) {
			fp = fopen(uap->filename, "r");
			break;
		}
	}
#endif /* EMBEDDED */
	
	if (fp == NULL)
		fp = open_auth_file(c->ctx, path);

	if (fp != NULL) {
		authorized = authorize(c, fp);
		(void) fclose(fp);
	}

	return (authorized);
}



#else  /*OPENWSMAN */


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
                return ctx->dauthf(ctx->realm, c->method, &di);
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


#endif
                

#ifndef NO_CGI
#ifdef _WIN32
struct threadparam {
	SOCKET	s;
	HANDLE	hPipe;
};

/*
 * Thread function that reads POST data from the socket pair
 * and writes it to the CGI process.
 */
static DWORD WINAPI
stdoutput(void *arg)
{
	struct threadparam	*tp = arg;
	int			n, k, sent, stop = 0;
	char			buf[BUFSIZ];

	while (!stop && (n = recv(tp->s, buf, sizeof(buf), 0)) > 0) {
		for (sent = 0; !stop && sent < n; sent += k)
			if (!WriteFile(tp->hPipe, buf + sent, n - sent, &k, 0))
				stop++;
	}
	
	CloseHandle(tp->hPipe);	/* Suppose we have POSTed everything */
	free(tp);
	return (0);
}

/*
 * Thread function that reads CGI output and pushes it to the socket pair.
 */
static DWORD WINAPI
stdinput(void *arg)
{
	struct threadparam	*tp = arg;
	static			int ntotal;
	int			n, k, sent, stop = 0;
	char			buf[BUFSIZ];

	while (!stop && ReadFile(tp->hPipe, buf, sizeof(buf), &n, NULL)) {
		ntotal += n;
		for (sent = 0; !stop && sent < n; sent += k)
			if ((k = send(tp->s, buf + sent, n - sent, 0)) <= 0)
				stop++;
	}
	elog(ERR_DEBUG, "stdinput: read %d bytes", ntotal);

	CloseHandle(tp->hPipe);
	
	/*
	 * Windows is a piece of crap. When this thread closes its end
	 * of the socket pair, the other end (get_cgi() function) may loose
	 * some data. I presume, this happens if get_cgi() is not fast enough,
	 * and the data written by this end does not "push-ed" to the other
	 * end socket buffer. So after closesocket() the remaining data is
	 * gone. If I put shutdown() before closesocket(), that seems to
	 * fix the problem, but I am not sure this is the right fix.
	 */
	shutdown(tp->s, 2);
	closesocket(tp->s);
	free(tp);
	return (0);
}

static int
spawn_stdio_thread(int sock, HANDLE hPipe, LPTHREAD_START_ROUTINE func)
{
	struct threadparam	*tp;
	DWORD			tid;

	tp = malloc(sizeof(*tp));
	assert(tp != NULL);

	tp->s		= sock;
	tp->hPipe	= hPipe;
	CreateThread(NULL, 0, func, (void *) tp, 0, &tid);

	return (0);
}
#endif /* _WIN32 */


#ifndef OPENWSMAN

/*
 * UNIX socketpair() implementation. Why? Because Windows does not have it.
 * Return 0 on success, -1 on error.
 */
static int
mysocketpair(int sp[2])
{
	struct sockaddr_in	sa;
	int			sock, ret = -1;
	socklen_t		len = sizeof(sa);

	(void) memset(&sa, 0, sizeof(sa));
	sa.sin_family 		= AF_INET;
	sa.sin_port		= htons(0);
	sa.sin_addr.s_addr	= htonl(INADDR_LOOPBACK);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		elog(ERR_INFO, "mysocketpair: socket(): %d", ERRNO);
	} else if (bind(sock, (struct sockaddr *) &sa, len) != 0) {
		elog(ERR_INFO, "mysocketpair: bind(): %d", ERRNO);
		(void) closesocket(sock);
	} else if (listen(sock, 1) != 0) {
		elog(ERR_INFO, "mysocketpair: listen(): %d", ERRNO);
		(void) closesocket(sock);
	} else if (getsockname(sock, (struct sockaddr *) &sa, &len) != 0) {
		elog(ERR_INFO, "mysocketpair: getsockname(): %d", ERRNO);
		(void) closesocket(sock);
	} else if ((sp[0] = socket(AF_INET, SOCK_STREAM, 6)) == -1) {
		elog(ERR_INFO, "mysocketpair: socket(): %d", ERRNO);
		(void) closesocket(sock);
	} else if (connect(sp[0], (struct sockaddr *) &sa, len) != 0) {
		elog(ERR_INFO, "mysocketpair: connect(): %d", ERRNO);
		(void) closesocket(sock);
		(void) closesocket(sp[0]);
	} else if ((sp[1] = accept(sock,(struct sockaddr *) &sa, &len)) == -1) {
		elog(ERR_INFO, "mysocketpair: accept(): %d", ERRNO);
		(void) closesocket(sock);
		(void) closesocket(sp[0]);
	} else {
		/* Success */
		ret = 0;
		(void) closesocket(sock);
	}

#ifndef _WIN32
	(void) fcntl(sp[0], F_SETFD, FD_CLOEXEC);
	(void) fcntl(sp[1], F_SETFD, FD_CLOEXEC);
#endif /* _WIN32*/

	return (ret);
}



/*
 * Spawn CGI program. Pass prepared environment to it.
 * Initialize c->fd descriptor to the IO socketpair end.
 * Return 0 if OK, -1 if error.
 */
static int
redirect(struct conn *c, char *interp, char *prog, char *envblk, char **envp)
{
	char			dir[FILENAME_MAX], *p;
	int			pair[2];
#ifdef _WIN32
	HANDLE			a[2], b[2], h[2], me;
	DWORD			flags;
	char			cmdline[FILENAME_MAX], line[FILENAME_MAX];
	FILE			*fp;
	STARTUPINFO		si;
	PROCESS_INFORMATION	pi;
#else
	pid_t			pid;
#endif /* _WIN32 */

	if (mysocketpair(pair) != 0)
		return (-1);

	/* CGI must be executed in its own directory */
	(void) strcpy(dir, prog);
	for (p = dir + strlen(dir) - 1; p > dir; p--)
		if (*p == '/') {
			*p++ = '\0';
			break;
		}
#ifdef _WIN32
	me = GetCurrentProcess();
	flags = DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS;

	/* FIXME add error checking code here */
	CreatePipe(&a[0], &a[1], NULL, 0);
	CreatePipe(&b[0], &b[1], NULL, 0);
	DuplicateHandle(me, a[0], me, &h[0], 0, TRUE, flags);
	DuplicateHandle(me, b[1], me, &h[1], 0, TRUE, flags);
	
	(void) memset(&si, 0, sizeof(si));
	(void) memset(&pi, 0, sizeof(pi));

	si.cb		= sizeof(si);
	si.dwFlags	= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.wShowWindow	= SW_HIDE;
	si.hStdOutput	= si.hStdError = h[1];
	si.hStdInput	= h[0];

	/* If CGI file is a script, try to read the interpreter line */
	if (interp == NULL) {
		if ((fp = fopen(prog, "r")) != NULL) {
			(void) fgets(line, sizeof(line), fp);
			if (memcmp(line, "#!", 2) != 0)
				line[2] = '\0';
			/* Trim whitespaces from interpreter name */
			for (p = &line[strlen(line) - 1]; p > line && isspace(*p); p--)
				*p = '\0';
			(void) fclose(fp);
		}
		(void) snprintf(cmdline, sizeof(cmdline), "%s%s%s",
		    line + 2, line[2] == '\0' ? "" : " ", prog);
	} else {
		(void) snprintf(cmdline, sizeof(cmdline), "%s %s",
		    interp, prog);
	}

	fix_directory_separators(dir);
	fix_directory_separators(cmdline);

	if (CreateProcess(NULL, cmdline, NULL, NULL, TRUE,
	    CREATE_NEW_PROCESS_GROUP, envblk, dir, &si, &pi) == 0) {
		elog(ERR_INFO,"redirect: CreateProcess(%s): %d",cmdline,ERRNO);
		return (-1);
	} else {
		CloseHandle(h[0]);
		CloseHandle(h[1]);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		spawn_stdio_thread(pair[1], b[0], stdinput);
		spawn_stdio_thread(pair[1], a[1], stdoutput);
		c->fd = pair[0];
	}
	return (0);
#else
	envblk = NULL;	/* unused */

	if ((pid = vfork()) == -1) {
		elog(ERR_INFO, "redirect: fork: %s", strerror(errno));
		(void) closesocket(pair[0]);
		(void) closesocket(pair[1]);
		return (-1);
	} else if (pid == 0) {
		/* Child */
		chdir(dir);
		(void) dup2(pair[1], 0);
		(void) dup2(pair[1], 1);
		(void) dup2(pair[1], 2);
		(void) closesocket(pair[0]);
		(void) closesocket(pair[1]);
		if (interp == NULL) {
			(void) execle(prog, prog, NULL, envp);
			elog(ERR_FATAL, "redirect: exec(%s)", prog);
		} else {
			(void) execle(interp, interp, prog, NULL, envp);
			elog(ERR_FATAL, "redirect: exec(%s %s)", interp, prog);
		}
	} else {
		/* Parent */
		(void) closesocket(pair[1]);
		c->fd = pair[0];
	}

	return (0);
#endif /* _WIN32 */
}


static void
addenv(char **env, int *len, char **penv, const char *fmt, const char *val)
{
	int	n;
	char	buf[ENV_MAX];

	n = Snprintf(buf, sizeof(buf), fmt, val);
	if (n > 0 && n < *len - 1) {
		*penv = *env;
		n++;	/* Include \0 terminator */
		(void) memcpy(*env, buf, n);
		(*env) += n;
		(*len) -= n;
	}
}

/*
 * Prepare the environment for the CGI program, and start CGI program.
 */
static int
spawncgi(struct conn *c, char *prog)
{
	char	env[ENV_MAX], *penv[64], var[IO_MAX], val[IO_MAX], hdr[IO_MAX],
		*s = env, *p, *p2;
	int	i, n = sizeof(env) - 1, k = 0;
	struct envvar *ep;

#define	ADDENV(x,y)	if (y) addenv(&s, &n, &penv[k++], x, y)

	/* Prepare the environment block */
	ADDENV("%s", "GATEWAY_INTERFACE=CGI/1.1");
	ADDENV("%s", "SERVER_PROTOCOL=HTTP/1.1");
	ADDENV("%s", "REDIRECT_STATUS=200");			/* For PHP */

	ADDENV("SERVER_PORT=%d", (char *) c->ctx->port);
	ADDENV("SERVER_NAME=%s", c->ctx->realm);
	ADDENV("SERVER_ROOT=%s", c->ctx->root);
	ADDENV("DOCUMENT_ROOT=%s", c->ctx->root);
	ADDENV("REQUEST_METHOD=%s", c->method);
	ADDENV("QUERY_STRING=%s", c->query);
	ADDENV("REMOTE_ADDR=%s", inet_ntoa(c->sa.u.sin.sin_addr));
	ADDENV("REMOTE_PORT=%u", (char *) ((int) ntohs(c->sa.u.sin.sin_port)));
	ADDENV("REQUEST_URI=%s", c->uri);
	ADDENV("SCRIPT_NAME=%s", prog + strlen(c->ctx->root));
	ADDENV("SCRIPT_FILENAME=%s", prog);			/* For PHP */
	ADDENV("PATH_TRANSLATED=%s", prog);
	ADDENV("CONTENT_TYPE=%s", c->ctype);
	ADDENV("CONTENT_LENGTH=%lu", (char *) c->cclength);
	ADDENV("PATH=%s", getenv("PATH"));
	ADDENV("COMSPEC=%s", getenv("COMSPEC"));		/* Win32 */
	ADDENV("SYSTEMROOT=%s", getenv("SYSTEMROOT"));		/* Win32 */
	ADDENV("LD_LIBRARY_PATH=%s", getenv("LD_LIBRARY_PATH"));
	ADDENV("PERLLIB=%s", getenv("PERLLIB"));
	ADDENV("PATH_INFO=/%s", c->path_info);

	if (c->user) {
		ADDENV("REMOTE_USER=%s", c->user);
		ADDENV("%s", "AUTH_TYPE=Digest");
	}

	/* Add user-specified variables */
	for (ep = c->ctx->envvars; ep != NULL; ep = ep->next) {
		(void) Snprintf(hdr, sizeof(hdr), "%s=%s", ep->name, ep->value);
		ADDENV("%s", hdr);
	}

	/* Add all headers as HTTP_* variables */
	for (p2 = strchr(c->remote.buf, '\n') + 1;
	    k < (int) NELEMS(penv) - 2 && n > 2 &&
	    p2 != NULL && p2[0] != '\n' && p2[0] != '\r'; p2 = p) {

		/* Remember the beginning of the next header */
		if ((p = strchr(p2, '\n')) != NULL)
			p++;

		/* Fetch header name and value */
		if (sscanf(p2, "%[-a-zA-Z0-9]: %[^\r\n]", var, val) != 2)
			continue;

		/* Upper-case header name. Convert - to _ */
		for (i = 0; i < (int) strlen(var); i++) {
			var[i] = toupper(((unsigned char *) var)[i]);
			if (var[i] == '-')
				var[i] = '_';
		}

		/* Add the header to the environment */
		(void) Snprintf(hdr, sizeof(hdr), "HTTP_%s=%s", var, val);
		ADDENV("%s", hdr);
	}

	penv[k] = NULL;
	env[sizeof(env) - n] = '\0';

	assert(k < (int) NELEMS(penv));
	assert(n > 0);
	assert(n < (int) sizeof(env));

	return (redirect(c, c->ctx->interp, prog, env, penv));
}

static void
cgiparse(struct conn *c)
{
	char	l[64];
	int	n;

	if ((n = getreqlen(c->local.buf, c->local.head)) == -1) {
		senderr(c, 500, "CGI Error", "", "Script sent invalid headers");
	} else if (n == 0) {
		/* Do not send anything to the client */
		c->local.tail = c->local.head;
	} else {
		/* Received all headers. Set status. */
		c->shlength = n;
		parse_headers(c, c->local.buf);
		if (c->location)
			c->status = 302;
		if (c->status == 0)
			c->status = 200;

		/* Output the status line */
		n = Snprintf(l, sizeof(l),  "HTTP/1.1 %u OK\r\n", c->status);
		(void) writeremote(c, l, n);

		/* Set flags, so script output will be passed directly out */
		c->flags |= FLAG_CGIPARSED;
		c->local.tail = 0;
	}
}

/*
 * GET or POST for cgi scripts
 */
static void
get_cgi(struct conn *c)
{
	int	n, len = IO_DATALEN(&c->remote);
	void	*buf = c->remote.buf + c->remote.tail;

	assert(c->fd != -1);

	/* Push data received from remote side to a CGI program */
	if ((c->flags & FLAG_FD_WRITABLE)) {
		if (c->http_method == METHOD_POST) {
			n = send(c->fd, buf, len, 0);
			if (n > 0) {
				elog(ERR_DEBUG, "get_cgi: %p: written %d",c,n);
				io_inc_tail(&c->remote, n);
			}
		} else {
			elog(ERR_DEBUG, "data sent with no POST method!");
			io_inc_tail(&c->remote, len);	/* Discard it */
		}
	}

	/*
	 * Script may output Status: and Location: headers,
	 * which may alter the response code. So buffer in headers,
	 * parse them, send correct status code and then forward
	 * all data from CGI script back to the remote end.
	 */
	if (c->flags & FLAG_FD_READABLE) {

		len = IO_SPACELEN(&c->local);
		buf = c->local.buf + c->local.head;

		if ((n = recv(c->fd, buf, len, 0)) > 0){
			c->local.head += n;
			if (!(c->flags & FLAG_CGIPARSED))
				cgiparse(c);
			elog(ERR_DEBUG, "get_cgi: %p: read %d", c, n);
		} else {
			c->local.done++;
			c->io = NULL;
			if (!(c->flags & FLAG_CGIPARSED))
				senderr(c, 500, "CGI Error", "",
				    "Bad headers sent:\n[%.*s]\n",
				c->local.head, c->local.buf);
		}
	}
}

/*
 * Verify that given file has CGI extension
 */
static int
iscgi(struct shttpd_ctx *ctx, const char *path)
{
	int		len1 = strlen(path), len2 = strlen(ctx->ext);
	return  (len1 > len2 && !ncasecmp(path + len1 - len2, ctx->ext, len2));
}
#endif /* NO_CGI */


#ifndef OPENWSMAN
/*
 * For given directory path, substitute it to valid index file.
 * Return 0 if index file has been found, -1 if not found
 */
static int
useindex(struct conn *c, char *path, size_t maxpath)
{
	struct stat	st;
	char		ftry[FILENAME_MAX], name[FILENAME_MAX];
	const char	*p, *s = c->ctx->index;
	size_t		len;

	do {
		if ((p = strchr(s, ',')) != NULL) {
			len = p - s;
			assert(len < sizeof(name));
			(void) memcpy(name, s, len);
			name[len] = '\0';
		} else {
			mystrlcpy(name, s, sizeof(name));
		}

		(void) snprintf(ftry, sizeof(ftry), "%s%c%s", path,DIRSEP,name);
		if (Stat(ftry, &st) == 0) {
			/* Found ! */
			mystrlcpy(path, ftry, maxpath);
			(void) strncat(c->uri, name,
			    sizeof(c->uri) - strlen(c->uri) - 1);
			c->st = st;
			return (0);
		}

		/* Move to the next index file */
		s = p ? p + 1 : NULL;
	} while (s != NULL);

	return (-1);
}
#endif // OPENWSMAN
#endif


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


#ifndef OPENWSMAN
/*
 * Try to open requested file, return 0 if OK, -1 if error.
 * If the file is given arguments using PATH_INFO mechanism,
 * initialize pathinfo pointer.
 */
static int
get_path_info(struct conn *c, char *path)
{
	char	*p, *e;

	if (Stat(path, &c->st) == 0)
		return (0);

	p = path + strlen(path);
	e = path + strlen(c->ctx->root) + 2;
	
	/* Strip directory parts of the path one by one */
	for (; p > e; p--)
		if (*p == '/') {
			*p = '\0';
			if (Stat(path, &c->st) == 0) {
				c->path_info = p + 1;
				return (0);
			} else {
				*p = '/';
			}
		}

	return (-1);
}

#endif

/*
 * Handle request
 */
static void
handle(struct conn *c)
{
	char			path[FILENAME_MAX + IO_MAX];
#ifndef OPENWSMAN
        char                    buf[1024];
	FILE			*fp = NULL;
#endif
	struct mountpoint	*mp;
	struct userurl		*userurl;

	elog(ERR_DEBUG, "handle: [%s]", c->remote.buf);

	if ((c->query = strchr(c->uri, '?')) != NULL) {
		*c->query++ = '\0';
#ifdef EMBEDDED
		c->query = c->http_method == METHOD_GET ?
			mystrdup(c->query) : NULL;
#else
		c->query = mystrdup(c->query);
#endif
	}
	urldecode(c->uri, c->uri);
	killdots(c->uri);
	(void) Snprintf(path, sizeof(path), "%s%s", c->ctx->root, c->uri);

	/* User may use the aliases - check URI for mount point */
	if ((mp = ismountpoint(c->ctx, c->uri)) != NULL) {
		(void) Snprintf(path, sizeof(path), "%s%s", mp->path,
		    c->uri + strlen(mp->mountpoint));
	}
	if (checkauth(c, path) != 1) {
		send_authorization_request(c);
	} else if ((userurl = isregistered(c->ctx, c->uri)) != NULL) {
		c->userurl = userurl;
		c->io = do_embedded;
		c->flags |= FLAG_ALWAYS_READY;
#ifndef OPENWSMAN
	} else if (strstr(path, HTPASSWD)) {
		senderr(c, 403, "Forbidden","", "Permission Denied");
	} else if ((c->http_method == METHOD_PUT || c->http_method == METHOD_DELETE) &&
			(c->ctx->put_auth == NULL ||
	    		(fp = fopen(c->ctx->put_auth, "r")) == NULL ||
			!authorize(c, fp))) {
		if (fp != NULL) {
			(void) fclose(fp);
		}
		send_authorization_request(c);
	} else if (c->http_method == METHOD_PUT) {
		int	rc;
		c->status = stat(path, &c->st) == 0 ? 200 : 201;

		if (c->range != NULL) {
			senderr(c, 501, "Not Implemented",
			    "","Range for PUT not implemented");
		} else if ((rc = put_dir(path)) == 0) {
			senderr(c, 200, "OK","","");
		} else if (rc == -1) {
			senderr(c, 500, "Error","","%s", strerror(errno));
		} else if (c->cclength == 0) {
			senderr(c, 411, "Length Required","","Length Required");
		} else if ((c->fd = Open(path, O_WRONLY | O_BINARY |
		    O_CREAT | O_NONBLOCK | O_TRUNC, 0644)) == -1) {
			elog(ERR_INFO, "handle: open(%s): %s",
			    path, strerror(errno));
			senderr(c, 500, "Error","","PUT error");
		} else {
			io_inc_tail(&c->remote, c->reqlen);
			c->io = put_file;
		}
	} else if (c->http_method == METHOD_DELETE) {
		if (remove(path) == 0)
			senderr(c, 200, "OK", "", "");
		else
			senderr(c, 500, "Error", "", "%s", strerror(errno));
	} else if (get_path_info(c, path) != 0) {
		senderr(c, 404, "Not Found","", "Not Found: [%s]", c->uri);
	} else if (S_ISDIR(c->st.st_mode) && path[strlen(path) - 1] != '/') {
		(void) snprintf(buf, sizeof(buf), "Location: %s/", c->uri);
		senderr(c, 301, "Moved Permanently", buf, "Moved, %s", buf);
	} else if (S_ISDIR(c->st.st_mode) &&
	    useindex(c, path, sizeof(path) - 1) == -1 && c->ctx->dirlist == 0) {
		senderr(c, 403, "Forbidden", "", "Directory Listing Denied");
	} else if (S_ISDIR(c->st.st_mode) && c->ctx->dirlist) {
		if ((c->path = mystrdup(path)) != NULL)
			do_dir(c);
		else
			senderr(c, 500, "Error", "", "strdup");
	} else if (S_ISDIR(c->st.st_mode) && c->ctx->dirlist == 0) {
		elog(ERR_INFO, "handle: %s: Denied", path);
		senderr(c, 403, "Forbidden", "", "Directory listing denied");
#ifndef NO_CGI
	} else if (iscgi(c->ctx, path)) {
		if ((spawncgi(c, path)) == -1) {
			senderr(c, 500, "Server Error", "", "Cannot exec CGI");
		} else {
			c->remote.tail = c->reqlen;
			io_inc_tail(&c->remote, 0);
			if (c->http_method == METHOD_GET)
				c->remote.head = 0;
			c->io = get_cgi;
			c->flags |= FLAG_CGI;
		}
#endif /* NO_CGI */
	} else if (c->ims && c->st.st_mtime <= c->ims) {
		senderr(c, 304, "Not Modified", "", "");
	} else if ((c->fd = Open(path, O_RDONLY | O_BINARY, 0644)) != -1) {
		do_get(c);
	} else {
		senderr(c, 500, "Error", "", "Internal Error");
		c->flags |= FLAG_FINISHED;
	}
#else // OPENWSMAN
        } else {
		senderr(c, 501, "Not Implemented", "", "Is not supported");
		c->flags |= FLAG_FINISHED;
	}
#endif // OPENWSMAN
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
	(void) strcpy(c->ouri, c->uri);	/* Save unmodified URI */
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
#if 0
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
			c->flags |= FLAG_FINISHED;
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
                (c->flags & (FLAG_FD_READABLE | FLAG_SOCK_READABLE))) {
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
#if 0
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
	SSL		*ssl = NULL;

	sa.len = sizeof(sa.u.sin);
	(void) nonblock(sock);

	if (getpeername(sock, &sa.u.sa, &sa.len)) {
		elog(ERR_INFO, "shttpd_add: %s", strerror(errno));
	} else if (ctx->ssl_ctx && (ssl = SSL_new(ctx->ssl_ctx)) == NULL) {
		elog(ERR_INFO, "shttpd_add: SSL_new: %s", strerror(ERRNO));
		(void) closesocket(sock);
	} else if (ctx->ssl_ctx && SSL_set_fd(ssl, sock) == 0) {
		elog(ERR_INFO, "shttpd_add: SSL_set_fd: %s", strerror(ERRNO));
		(void) closesocket(sock);
		SSL_free(ssl);
	} else if ((c = calloc(1, sizeof(*c))) == NULL) {
		(void) closesocket(sock);
		elog(ERR_INFO, "shttpd_add: calloc: %s", strerror(ERRNO));
	} else {
		c->sa		= sa;
		c->sock		= sock;
		c->watch	= serve;
		c->watch_data	= c;
		c->fd		= -1;
		c->ssl		= ssl;
		c->birth	= current_time;
		c->expire	= current_time + EXPIRE_TIME;
#ifndef _WIN32
		(void) fcntl(sock, F_SETFD, FD_CLOEXEC);
#endif /* _WIN32 */
		if (ssl)
			handshake(c);
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

	sock = inetd ? fileno(stdin) : accept((int) ptr, &sa.u.sa, &sa.len);	
	if (sock == -1)
		elog(ERR_INFO, "do_accept(%d): %s", (int) ptr, strerror(ERRNO));
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
	c->watch_data	= (void *) sock;
	c->sock		= sock;
	c->fd		= -1;
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
	do {FD_SET(fd, set); if (fd > max_fd) max_fd = fd; } while (0)
	
		/* If there is space to read, do it */
		if (IO_SPACELEN(&c->remote))
			MERGEFD(c->sock, &read_set);

		/* If there is data in local buffer, add to write set */
		if (IO_DATALEN(&c->local)) {
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
#ifdef _WIN32
		/*
		 * In windows, if read_set and write_set are empty,
		 * select() returns "Invalid parameter" error
		 * (at least on my Windows XP Pro). So in this case,
		 * we sleep here.
		 */
		Sleep(milliseconds);
#else
		elog(ERR_DEBUG, "select: %s", strerror(errno));
#endif /* _WIN32 */
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
 * Show usage string and exit.
 */
static void
usage(const char *prog)
{
	struct opt	*opt;

	(void) fprintf(stderr,
	    "SHTTPD version %s (c) Sergey Lyubka\n"
	    "usage: %s [OPTIONS] [config_file]\n"
	    "Note: config line keyword for every option is in the "
	    "round brackets\n", SHTTPD_VERSION, prog);
	(void) fprintf(stderr, "-A <htpasswd_file> <realm> <user> <passwd>\n");

	for (opt = options; opt->name != NULL; opt++)
		(void) fprintf(stderr, "-%c <%s>\t\t%s (%s)\n",
		    opt->sw, opt->arg, opt->desc, opt->name);

	exit(EXIT_FAILURE);
}

/*
 * Initialize shttpd
 */
struct shttpd_ctx *
do_init(const char *config_file, int argc, char *argv[])
{
	struct shttpd_ctx	*ctx;
	char		line[FILENAME_MAX],var[sizeof(line)],val[sizeof(line)];
	const char	*arg;
	int		i;
	struct opt	*opt;
	FILE 		*fp;

	current_time = time(NULL);

	if ((ctx = calloc(1, sizeof(*ctx))) == NULL)
		elog(ERR_FATAL, "do_init: cannot allocate context");

	ctx->start_time = current_time;
	InitializeCriticalSection(&ctx->mutex);

	/*
	 * First pass: set the defaults.
	 * setopt() may already set some vars, we do not override them
	 * with the default value if they are set.
	 */
	for (opt = options; opt->sw != 0; opt++)
		if (opt->tmp == NULL && opt->def != NULL)
			opt->tmp = strdup(opt->def);

	/* Second pass: load config file  */
	if (config_file != NULL && (fp = fopen(config_file, "r")) != NULL) {
		elog(ERR_DEBUG, "using config file %s", config_file);

		/* Loop through the lines in config file */
		while (fgets(line, sizeof(line), fp) != NULL) {

			/* Skip comments and empty lines */
			if (line[0] == '#' || line[0] == '\n')
				continue;

			/* Trim trailing newline character */
			line[strlen(line) - 1] = '\0';
			if (sscanf(line, "%s %s", var, val) != 2)
				elog(ERR_FATAL, "do_init: bad line: [%s]",line);

			if ((opt = swtoopt(0, var)) == NULL) {
			//	elog(ERR_FATAL, "Unknown variable [%s]", var);
			} else {
				if (opt->tmp)
					free(opt->tmp);
				opt->tmp = strdup(val);
			}
		}
		(void) fclose(fp);
	}

	/* Third pass: process command line args */
	for (i = 1; i < argc && argv[i][0] == '-'; i++)
#ifndef EMBEDDED
		if (argv[i][1] == 'A') {
			/* Option 'A' require special handling */
			if (argc != 6)
				usage(argv[0]);
			exit(editpass(argv[2], argv[3], argv[4], argv[5]));
		} else
#endif /* !EMBEDDED */
		if ((opt = swtoopt(argv[i][1], NULL)) != NULL) {
			if (opt->tmp != NULL)
				free(opt->tmp);
			arg = argv[i][2] ? &argv[i][2] : argv[++i];
			if (arg == NULL)
				usage(argv[0]);
			opt->tmp = strdup(arg);
		} else {
			usage(argv[0]);
		}

	/* Now, every option must hold its config in 'tmp' node. Call setters */
	for (opt = options; opt->sw != 0; opt++)
		if (opt->tmp != NULL)
			opt->setter(ctx, ((char *) ctx) + opt->ofs, opt->tmp);

	/* If SSL is wanted and port is not overridden, set it to 443 */
	if (ctx->port == atoi(PORT) && ctx->ssl_ctx != NULL)
		ctx->port = 443;

	/* If document_root is not set, set it to current directory */
	if (ctx->root[0] == '\0')
		(void) getcwd(ctx->root, sizeof(ctx->root));

#ifdef _WIN32
	{WSADATA data;	WSAStartup(MAKEWORD(2,2), &data);}
#endif /* _WIN32 */


	elog(ERR_DEBUG, "do_init: initialized context %p", ctx);

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

#ifndef EMBEDDED
/*
 * SIGTERM, SIGINT signal handler
 */
static void
sigterm(int signo)
{
	exit_flag = signo;
}

/*
 * Grim reaper of innocent children: SIGCHLD signal handler
 */
static void
sigchild(int signo)
{
	while (waitpid(-1, &signo, WNOHANG) > 0) ;
}


#ifndef NO_GUI
/*
 * Dialog box control IDs
 */
#define ID_GROUP	100
#define ID_SAVE		101
#define	ID_STATUS	102
#define	ID_STATIC	103
#define	ID_SETTINGS	104
#define	ID_QUIT		105
#define	ID_TRAYICON	106
#define	ID_TIMER	107
#define	ID_ICON		108
#define	ID_USER		200
#define	ID_DELTA	1000

static DWORD WINAPI
thread_function(void *param)
{
	struct shttpd_ctx	*ctx = param;
	int			sock;

	if ((sock = shttpd_open_port(ctx->port)) == -1)
		elog(ERR_FATAL, "Cannot open socket on port %d", ctx->port);
	shttpd_listen(ctx, sock);

	while (WaitForSingleObject(ctx->ev[0], 0) != WAIT_OBJECT_0)
		shttpd_poll(ctx, 1000);

	SetEvent(ctx->ev[1]);
	shttpd_fini(ctx);

	return (0);
}

/*
 * Save the configuration back into config file
 */
static void
save_config(HWND hDlg, FILE *fp)
{
	struct opt	*opt;
	char		text[FILENAME_MAX];
	int		id;

	if (fp == NULL)
		elog(ERR_FATAL, "save_config: cannot open %s", config_file);

	for (opt = options; opt->name != NULL; opt++) {
		id = ID_USER + (opt - options);		/* Control ID */

		if (opt->flags & OPT_FLAG_BOOL)
			(void) fprintf(fp, "%s\t%d\n",
			    opt->name, IsDlgButtonChecked(hDlg, id));
		else if (GetDlgItemText(hDlg, id, text, sizeof(text)) != 0)
			(void) fprintf(fp, "%s\t%s\n", opt->name, text);
	}

	(void) fclose(fp);
}

/*
 * The dialog box procedure.
 */
BOOL CALLBACK
DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND	hStatus;
	static struct shttpd_ctx *ctx, **pctx;
	HANDLE		ev;
	struct opt	*opt;
	int		id, sock, tid, opt_index,
				up, widths[] = {120, 210, 370, -1};
	char		text[256];

	switch (msg) {

	case WM_CLOSE:
		KillTimer(hDlg, ID_TIMER);
		DestroyWindow(hDlg);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_SAVE:
			EnableWindow(GetDlgItem(hDlg, ID_SAVE), FALSE);
			SendMessage(hStatus,SB_SETTEXT,3,(LPARAM)" Saving...");
			save_config(hDlg, fopen(config_file, "w+"));
			ev = ctx->ev[1];
			SetEvent(ctx->ev[0]);
			SendMessage(hStatus,SB_SETTEXT,3,
				(LPARAM) " Restaring...");
			WaitForSingleObject(ev, INFINITE);
			*pctx = ctx = do_init(config_file, Argc, Argv);
			sock = shttpd_open_port(ctx->port);
			shttpd_listen(ctx, sock);
			CreateThread(NULL, 0, thread_function, ctx, 0, &tid);
			SendMessage(hStatus, SB_SETTEXT, 3, (LPARAM)" Running");
			EnableWindow(GetDlgItem(hDlg, ID_SAVE), TRUE);

			break;
		}

		id = ID_USER + ID_DELTA;
		for (opt = options; opt->name != NULL; opt++, id++)
			if (LOWORD(wParam) == id) {
				OPENFILENAME	of;
				BROWSEINFO	bi;
				char		path[FILENAME_MAX] = "";

				memset(&of, 0, sizeof(of));
				of.lStructSize = sizeof(of);
				of.hwndOwner = (HWND) hDlg;
				of.lpstrFile = path;
				of.nMaxFile = sizeof(path);
				of.lpstrInitialDir = ctx->root;
				of.Flags = OFN_CREATEPROMPT | OFN_NOCHANGEDIR;
				
				memset(&bi, 0, sizeof(bi));
				bi.hwndOwner = (HWND) hDlg;
				bi.lpszTitle = "Choose WWW root directory:";
				bi.ulFlags = BIF_RETURNONLYFSDIRS;

				if (opt->flags & OPT_FLAG_DIR)
					SHGetPathFromIDList(
						SHBrowseForFolder(&bi), path);
				else
					GetOpenFileName(&of);

				if (path[0] != '\0')
					SetWindowText(GetDlgItem(hDlg,
						id - ID_DELTA), path);
			}

		break;

	case WM_TIMER:
		/* Print statistics on a status bar */
		up = current_time - ctx->start_time;
		(void) snprintf(text, sizeof(text),
		    " Up: %3d h %2d min %2d sec",
		    up / 3600, up / 60 % 60, up % 60);
		SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) text);
		(void) snprintf(text, sizeof(text),
		    " Requests: %u", ctx->nrequests);
		SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM) text);
		(void) snprintf(text, sizeof(text),
		    " In: %.2f Mb, Out: %.2f Mb",
		    (double) ctx->kb_in / 1024, (double) ctx->kb_out / 1024);
		SendMessage(hStatus, SB_SETTEXT, 2, (LPARAM) text);
		break;

	case WM_INITDIALOG:
		pctx = (struct shttpd_ctx **) lParam;
		ctx = *pctx;
		SendMessage(hDlg,WM_SETICON,(WPARAM)ICON_SMALL,(LPARAM)hIcon);
		SendMessage(hDlg,WM_SETICON,(WPARAM)ICON_BIG,(LPARAM)hIcon);
		hStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE,
			"", hDlg, ID_STATUS);
		SendMessage(hStatus, SB_SETPARTS, 4, (LPARAM) widths);
		SendMessage(hStatus, SB_SETTEXT, 3, (LPARAM) " Running");
		SetWindowText(hDlg, "SHTTPD settings");
		SetFocus(GetDlgItem(hDlg, ID_SAVE));
		SetTimer(hDlg, ID_TIMER, 1000, NULL);

		for (opt = options; opt->name != NULL; opt++) {
			opt_index = opt - options;
			if (opt->flags & OPT_FLAG_BOOL)
				CheckDlgButton(hDlg, ID_USER + opt_index,
			opt->tmp[0] == '0' ? BST_UNCHECKED : BST_CHECKED);
			else
				SetDlgItemText(hDlg, ID_USER + opt_index,
					opt->tmp);
		}
		break;
	default:
		break;
	}

	return FALSE;
}

static void *
align(void *ptr, DWORD alig)
{
	ULONG ul = (ULONG) ptr;

	ul += alig;
	ul &= ~alig;
	
	return ((void *) ul);
}


static void
add_control(char **mem, DLGTEMPLATE *dia, DWORD type, DWORD id, DWORD style,
	WORD x, WORD y, WORD cx, WORD cy, const char *caption)
{
	DLGITEMTEMPLATE	*tp;
	LPWORD		p;

	dia->cdit++;

	*mem = align(*mem, 3);
	tp = (DLGITEMTEMPLATE *) *mem;

	tp->id			= id;
	tp->style		= style;
	tp->dwExtendedStyle	= 0;
	tp->x			= x;
	tp->y			= y;
	tp->cx			= cx;
	tp->cy			= cy;

	p = align(*mem + sizeof(*tp), 1);
	*p++ = 0xffff;
	*p++ = type;

	while (*caption != '\0')
		*p++ = (WCHAR) *caption++;
	*p++ = 0;
	p = align(p, 1);

	*p++ = 0;
	*mem = (char *) p;
}

static void
show_settings_dialog(struct shttpd_ctx **ctxp)
{
#define	HEIGHT		15
#define	WIDTH		400
#define	LABEL_WIDTH	70

	unsigned char	mem[4096], *p;
	DWORD		cl, style;
	DLGTEMPLATE	*dia = (DLGTEMPLATE *) mem;
	WORD		x, y, cx, width, nelems = 0;
	struct opt	*opt;
	struct {
		DLGTEMPLATE	template;	/* 18 bytes */
		WORD		menu, class;
		wchar_t		caption[1];
		WORD		fontsiz;
		wchar_t		fontface[7];
	} dialog_header = {{WS_CAPTION | WS_POPUP | WS_SYSMENU | WS_VISIBLE |
		DS_SETFONT | WS_MINIMIZEBOX | WS_DLGFRAME,
		0, 0, 200, 200, WIDTH, 0}, 0, 0, L"", 8, L"Tahoma"};

	(void) memset(mem, 0, sizeof(mem));
	(void) memcpy(mem, &dialog_header, sizeof(dialog_header));
	p = mem + sizeof(dialog_header);

	for (opt = options; opt->name != NULL; opt++) {
		style = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
		x = 10 + (WIDTH / 2) * (nelems % 2);
		y = (nelems/2 + 1) * HEIGHT + 5;
		width = WIDTH / 2 - 20 - LABEL_WIDTH;
		if (opt->flags & OPT_FLAG_INT) {
			style |= ES_NUMBER;
			cl = 0x81;
			style |= WS_BORDER | ES_AUTOHSCROLL;
		} else if (opt->flags & OPT_FLAG_BOOL) {
			cl = 0x80;
			style |= BS_AUTOCHECKBOX;
		} else if (opt->flags & (OPT_FLAG_DIR | OPT_FLAG_FILE)) {
			style |= WS_BORDER | ES_AUTOHSCROLL;
			width -= 20;
			cl = 0x81;
			add_control(&p, dia, 0x80,
				ID_USER + ID_DELTA + (opt - options),
				WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
				(WORD) (x + width + LABEL_WIDTH + 5),
				y, 15, 12, "...");
		} else {
			cl = 0x81;
			style |= WS_BORDER | ES_AUTOHSCROLL;
		}
		add_control(&p, dia, 0x82, ID_STATIC, WS_VISIBLE | WS_CHILD,
			x, y, LABEL_WIDTH, HEIGHT, opt->desc);
		add_control(&p, dia, cl, ID_USER + (opt - options), style,
			(WORD) (x + LABEL_WIDTH), y, width, 12, "");
		nelems++;
	}

	y = (WORD) (((nelems + 1)/2 + 1) * HEIGHT + 5);
	add_control(&p, dia, 0x80, ID_GROUP, WS_CHILD | WS_VISIBLE |
		BS_GROUPBOX, 5, 5, WIDTH - 10, y, "Settings");
	y += 10;
	add_control(&p, dia, 0x80, ID_SAVE,
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		WIDTH - 50, y, 45, 12, "Save");
	add_control(&p, dia, 0x82, ID_STATIC,
		WS_CHILD | WS_VISIBLE | WS_DISABLED,
		5, y, WIDTH - 60, 12,"SHTTPD v." SHTTPD_VERSION
		"      (http://shttpd.sourceforge.net)");
	
	dia->cy = ((nelems + 1)/2 + 1) * HEIGHT + 40;
	DialogBoxIndirectParam(NULL, dia, NULL, DlgProc, (LPARAM) ctxp);
}

static LRESULT CALLBACK
WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND		hDlg;
	static NOTIFYICONDATA	ni;
	static struct shttpd_ctx *ctx;
	DWORD			tid;		/* Thread ID */
	HMENU			hMenu;
	POINT			pt;

	switch (msg) {
	case WM_CREATE:
		ctx = ((CREATESTRUCT *) lParam)->lpCreateParams;
		(void) memset(&ni, 0, sizeof(ni));
		ni.cbSize = sizeof(ni);
		ni.uID = ID_TRAYICON;
		ni.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		ni.hIcon = hIcon;
		ni.hWnd = hWnd;
		(void) snprintf(ni.szTip, sizeof(ni.szTip),"SHTTPD web server");
		ni.uCallbackMessage = WM_USER;
		Shell_NotifyIcon(NIM_ADD, &ni);
		ctx->ev[0] = CreateEvent(0, TRUE, FALSE, 0);
		ctx->ev[1] = CreateEvent(0, TRUE, FALSE, 0);
		CreateThread(NULL, 0, thread_function, ctx, 0, &tid);
		break;
	case WM_CLOSE:
		Shell_NotifyIcon(NIM_DELETE, &ni);
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_SETTINGS:
			show_settings_dialog(&ctx);
			break;
		case ID_QUIT:
			SendMessage(hWnd, WM_CLOSE, wParam, lParam);
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_USER:
		switch (lParam) {
		case WM_RBUTTONUP:
			hMenu = CreatePopupMenu();
			AppendMenu(hMenu, 0, ID_SETTINGS, "Settings");
			AppendMenu(hMenu, 0, ID_QUIT, "Exit SHTTPD");
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, hWnd, NULL);
			DestroyMenu(hMenu);
			break;
		}
		break;
	}

	return (DefWindowProc(hWnd, msg, wParam, lParam));
}

static void
run_gui(struct shttpd_ctx *ctx)
{
	WNDCLASS	cls;
	HWND		hWnd;
	MSG		msg;

	(void) FreeConsole();
	(void) memset(&cls, 0, sizeof(cls));

	hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_ICON));
	cls.lpfnWndProc = (WNDPROC) WindowProc; 
	cls.hIcon = hIcon;
	cls.lpszClassName = "shttpd v." SHTTPD_VERSION; 

	if (!RegisterClass(&cls)) 
		elog(ERR_FATAL, "run_gui: RegisterClass: %d", ERRNO);
	else if ((hWnd = CreateWindow(cls.lpszClassName, "",WS_OVERLAPPEDWINDOW,
	    0, 0, 0, 0, NULL, NULL, NULL, ctx)) == NULL)
		elog(ERR_FATAL, "run_gui: CreateWindow: %d", ERRNO);

	while (GetMessage(&msg, (HWND) NULL, 0, 0)) { 
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
	}

	exit(0);
}
#endif /* NO_GUI */

int
main(int argc, char *argv[])
{
	struct shttpd_ctx	*ctx;
	current_time = time(NULL);
	config_file = CONFIG;
	if (argc > 1 && argv[argc - 2][0] != '-' && argv[argc - 1][0] != '-')
		config_file = argv[argc - 1];

	ctx = do_init(config_file, argc, argv);

#ifdef _WIN32
	Argc = argc;
	Argv = argv;
	if (ctx->gui)
		run_gui(ctx);
#endif /* _WIN32 */

	if (inetd)
		shttpd_add(ctx, fileno(stdin));
	else
		shttpd_listen(ctx, shttpd_open_port(ctx->port));

	/* Switch to alternate UID, it is safe now, after shttpd_open_port() */
#ifndef _WIN32
	if (ctx->uid != NULL) {
		struct passwd	*pw;

		if ((pw = getpwnam(ctx->uid)) == NULL)
			elog(ERR_FATAL, "main: unknown user [%s]", ctx->uid);
		else if (setgid(pw->pw_gid) == -1)
			elog(ERR_FATAL, "main: setgid(%s): %s",
			    ctx->uid, strerror(errno));
		else if (setuid(pw->pw_uid) == -1)
			elog(ERR_FATAL, "main: setuid(%s): %s",
			    ctx->uid, strerror(errno));
	}
	(void) signal(SIGCHLD, sigchild);
	(void) signal(SIGPIPE, SIG_IGN);
#endif /* _WIN32 */

	(void) signal(SIGTERM, sigterm);
	(void) signal(SIGINT, sigterm);

	elog(ERR_INFO, "shttpd %s started on port %d, serving %s",
	    SHTTPD_VERSION, ctx->port, ctx->root);

	while (exit_flag == 0)
		shttpd_poll(ctx, 5000);

	elog(ERR_INFO, "%d requests, %u Kb in, %u Kb out. Exit on signal %d",
	    ctx->nrequests, ctx->kb_in, ctx->kb_out, exit_flag);

	shttpd_fini(ctx);

	return (EXIT_SUCCESS);
}
#endif /* !EMBEDDED */
