/* wsman_config.h.cmake.  Copied from wsman_config.h.in  */

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
#if @CRAY_STACKSEG_END@
#define CRAY_STACKSEG_END 1
#endif

/* Define to 1 if using `alloca.c'. */
#if @C_ALLOCA@
#define C_ALLOCA 1
#endif

/* Default server config file */
#define DEFAULT_CONFIG_FILE @DEFAULT_CONFIG_FILE@

/* Default client config file */
#define DEFAULT_CLIENT_CONFIG_FILE @DEFAULT_CLIENT_CONFIG_FILE@

/* Defined if WS-Eventing wanted */
#if @ENABLE_EVENTING_SUPPORT@
#define ENABLE_EVENTING_SUPPORT 1
#endif

/* Define to 1 if you have `alloca', as a function or macro. */
#if @HAVE_ALLOCA@
#define HAVE_ALLOCA 1
#endif

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
#if @HAVE_ALLOCA_H@
#define HAVE_ALLOCA_H 1
#endif

/* Define to 1 if you have the `bcopy' function. */
#if @HAVE_BCOPY@
#define HAVE_BCOPY 1
#endif

/* Define to 1 if you have the `crypt' function. */
#if @HAVE_CRYPT@
#define HAVE_CRYPT 1
#endif

/* Define to 1 if you have the <crypt.h> header file. */
#if @HAVE_CRYPT_H@
#define HAVE_CRYPT_H 1
#endif

/* Define to 1 if you have the <ctype.h> header file. */
#if @HAVE_CTYPE_H@
#define HAVE_CTYPE_H 1
#endif

/* Define to 1 if you have the <CUnit/Basic.h> header file. */
#if @HAVE_CUNIT_BASIC_H@
#define HAVE_CUNIT_BASIC_H 1
#endif

/* Define to 1 if you have the `daemon' function. */
#if @HAVE_DAEMON@
#define HAVE_DAEMON 1
#endif

/* Define to 1 if you have the <dirent.h> header file. */
#if @HAVE_DIRENT_H@
#define HAVE_DIRENT_H 1
#endif

/* Define to 1 if you have the <dlfcn.h> header file. */
#if @HAVE_DLFCN_H@
#define HAVE_DLFCN_H 1
#endif

/* Define to 1 if you have the `fnmatch' function. */
#if @HAVE_FNMATCH@
#define HAVE_FNMATCH 1
#endif

/* Define to 1 if you have the `getaddrinfo' function. */
#if @HAVE_GETADDRINFO@
#define HAVE_GETADDRINFO 1
#endif

/* Define to 1 if you have the `getnameinfo' function. */
#if @HAVE_GETNAMEINFO@
#define HAVE_GETNAMEINFO 1
#endif

/* Define to 1 if you have the `getpid' function. */
#if @HAVE_GETPID@
#define HAVE_GETPID 1
#endif

/* Define to 1 if you have the `gettimeofday' function. */
#if @HAVE_GETTIMEOFDAY@
#define HAVE_GETTIMEOFDAY 1
#endif

/* Define to 1 if you have the `gmtime_r' function. */
#if @HAVE_GMTIME_R@
#define HAVE_GMTIME_R 1
#endif

/* Define to 1 if you have the `inet_aton' function. */
#if @HAVE_INET_ATON@
#define HAVE_INET_ATON 1
#endif

/* Define to 1 if you have the `inet_ntop' function. */
#if @HAVE_INET_NTOP@
#define HAVE_INET_NTOP 1
#endif

/* Define to 1 if you have the `inet_pton' function. */
#if @HAVE_INET_PTON@
#define HAVE_INET_PTON 1
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#if @HAVE_INTTYPES_H@
#define HAVE_INTTYPES_H 1
#endif

/* Define to 1 if the system has the type `in_addr_t'. */
#if @HAVE_IN_ADDR_T@
#define HAVE_IN_ADDR_T 1
#endif

/* Define to 1 if the system has the type `in_port_t'. */
#if @HAVE_IN_PORT_T@
#define HAVE_IN_PORT_T 1
#endif

/* libcrypt library present */
#if @HAVE_LIBCRYPT@
#define HAVE_LIBCRYPT 1
#endif

/* Define to 1 if you have the `nsl' library (-lnsl). */
#if @HAVE_LIBNSL@
#define HAVE_LIBNSL 1
#endif

/* Define to 1 if you have the `socket' library (-lsocket). */
#if @HAVE_LIBSOCKET@
#define HAVE_LIBSOCKET 1
#endif

/* Define to 1 if you have the `memmove' function. */
#if @HAVE_MEMMOVE@
#define HAVE_MEMMOVE 1
#endif

/* Define to 1 if you have the <memory.h> header file. */
#if @HAVE_MEMORY_H@
#define HAVE_MEMORY_H 1
#endif

/* Define to 1 if you have the <netinet/in.h> header file. */
#if @HAVE_NETINET_IN_H@
#define HAVE_NETINET_IN_H 1
#endif

/* Define to 1 if you have the <net/if_dl.h> header file. */
#if @HAVE_NET_IF_DL_H@
#define HAVE_NET_IF_DL_H 1
#endif

/* Define to 1 if you have the <net/if.h> header file. */
#if @HAVE_NET_IF_H@
#define HAVE_NET_IF_H 1
#endif

/* Defined if pam support is available */
#if @HAVE_PAM@
#define HAVE_PAM 1
#endif

/* Define to 1 if you have the <pam/pam_appl.h> header file. */
#if @HAVE_PAM_PAM_APPL_H@
#define HAVE_PAM_PAM_APPL_H 1
#endif

/* Define to 1 if you have the <pam/pam_misc.h> header file. */
#if @HAVE_PAM_PAM_MISC_H@
#define HAVE_PAM_PAM_MISC_H 1
#endif

/* Define to 1 if you have the <pthread.h> header file. */
#if @HAVE_PTHREAD_H@
#define HAVE_PTHREAD_H 1
#endif

/* Define to 1 if the system has the type `sa_family_t'. */
#if @HAVE_SA_FAMILY_T@
#define HAVE_SA_FAMILY_T 1
#endif

/* Define if struct sockaddr contains sa_len */
#if @HAVE_SA_LEN@
#define HAVE_SA_LEN 1
#endif

/* Define to 1 if you have the <security/pam_appl.h> header file. */
#if @HAVE_SECURITY_PAM_APPL_H@
#define HAVE_SECURITY_PAM_APPL_H 1
#endif

/* Define to 1 if you have the <security/pam_misc.h> header file. */
#if @HAVE_SECURITY_PAM_MISC_H@
#define HAVE_SECURITY_PAM_MISC_H 1
#endif

/* Define to 1 if you have the `sleep' function. */
#if @HAVE_SLEEP@
#define HAVE_SLEEP 1
#endif

/* Define to 1 if you have the `srandom' function. */
#if @HAVE_SRANDOM@
#define HAVE_SRANDOM 1
#endif

/* Defined if you have SSL support */
#if @HAVE_SSL@
#define HAVE_SSL 1
#endif

/* Define to 1 if you have the <stdarg.h> header file. */
#if @HAVE_STDARG_H@
#define HAVE_STDARG_H 1
#endif

/* Define to 1 if you have the <stdint.h> header file. */
#if @HAVE_STDINT_H@
#define HAVE_STDINT_H 1
#endif

/* Define to 1 if you have the <stdlib.h> header file. */
#if @HAVE_STDLIB_H@
#define HAVE_STDLIB_H 1
#endif

/* Define to 1 if you have the <strings.h> header file. */
#if @HAVE_STRINGS_H@
#define HAVE_STRINGS_H 1
#endif

/* Define to 1 if you have the <string.h> header file. */
#if @HAVE_STRING_H@
#define HAVE_STRING_H 1
#endif

/* Define to 1 if you have the `strsep' function. */
#if @HAVE_STRSEP@
#define HAVE_STRSEP 1
#endif

/* Define to 1 if you have the `strtok_r' function. */
#if @HAVE_STRTOK_R@
#define HAVE_STRTOK_R 1
#endif

/* Define to 1 if you have the `syslog' function. */
#if @HAVE_SYSLOG@
#define HAVE_SYSLOG 1
#endif

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#if @HAVE_SYS_IOCTL_H@
#define HAVE_SYS_IOCTL_H 1
#endif

/* Define to 1 if you have the <sys/resource.h> header file. */
#if @HAVE_SYS_RESOURCE_H@
#define HAVE_SYS_RESOURCE_H 1
#endif

/* Define to 1 if you have the <sys/select.h> header file. */
#if @HAVE_SYS_SELECT_H@
#define HAVE_SYS_SELECT_H 1
#endif

/* Define to 1 if you have the <sys/sendfile.h> header file. */
#if @HAVE_SYS_SENDFILE_H@
#define HAVE_SYS_SENDFILE_H 1
#endif

/* Define to 1 if you have the <sys/signal.h> header file. */
#if @HAVE_SYS_SIGNAL_H@
#define HAVE_SYS_SIGNAL_H 1
#endif

/* Define to 1 if you have the <sys/socket.h> header file. */
#if @HAVE_SYS_SOCKET_H@
#define HAVE_SYS_SOCKET_H 1
#endif

/* Define to 1 if you have the <sys/sockio.h> header file. */
#if @HAVE_SYS_SOCKIO_H@
#define HAVE_SYS_SOCKIO_H 1
#endif

/* Define to 1 if you have the <sys/stat.h> header file. */
#if @HAVE_SYS_STAT_H@
#define HAVE_SYS_STAT_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#if @HAVE_SYS_TYPES_H@
#define HAVE_SYS_TYPES_H 1
#endif

/* Define to 1 if you have the `timegm' function. */
#if @HAVE_TIMEGM@
#define HAVE_TIMEGM 1
#endif

/* Define to 1 if you have the <unistd.h> header file. */
#if @HAVE_UNISTD_H@
#define HAVE_UNISTD_H 1
#endif

/* Define to 1 if you have the `unlink' function. */
#if @HAVE_UNLINK@
#define HAVE_UNLINK 1
#endif

/* Define to 1 if you have the <vararg.h> header file. */
#if @HAVE_VARARG_H@
#define HAVE_VARARG_H 1
#endif

/* Define to 1 if you have the `va_copy' function. */
#if @HAVE_VA_COPY@
#define HAVE_VA_COPY 1
#endif

/* Name of package */
#define PACKAGE @PACKAGE@

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT @PACKAGE_BUGREPORT@

/* Define to the full name of this package. */
#define PACKAGE_NAME @PACKAGE_NAME@

/* Define to the full name and version of this package. */
#define PACKAGE_STRING @PACKAGE_STRING@

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME @PACKAGE_TARNAME@

/* Define to the version of this package. */
#define PACKAGE_VERSION @PACKAGE_VERSION@

/* Define to the API version of the server plugin interface. */
#define OPENWSMAN_PLUGIN_API_VERSION @OPENWSMAN_PLUGIN_API_VERSION@

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT @SIZEOF_INT@

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG @SIZEOF_LONG@

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG @SIZEOF_LONG_LONG@

/* The size of `short', as computed by sizeof. */
#define SIZEOF_SHORT @SIZEOF_SHORT@

/* Define to 1 if you have the ANSI C header files. */
#if @STDC_HEADERS@
#define STDC_HEADERS 1
#endif

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#if @TIME_WITH_SYS_TIME@
#define TIME_WITH_SYS_TIME 1
#endif

/* Version number of package */
#define VERSION @VERSION@

/* Defined if verbose debug logging is requested */
#if @WSMAN_DEBUG_VERBOSE@
#define WSMAN_DEBUG_VERBOSE 1
#endif

/* Define to `int' if <sys/types.h> does not define. */
#if @SSIZE_T_MISSING@
#define ssize_t int
#endif

/* Define to 1 if you want to enable IPv6 support. */
#if @ENABLE_IPV6@
#define ENABLE_IPV6 1
#endif
