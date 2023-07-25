 *******************************************
dnl *** Initialize automake and set version ***
dnl *******************************************

AC_PREREQ(2.53)
AC_INIT([openwsman], [2.7.2])
AC_CONFIG_SRCDIR(src/server/wsmand.c)
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])

dnl Package version information
dnl See VERSION.cmake for correct version numbers
PACKAGE=openwsman
OPENWSMAN_MAJOR=2
OPENWSMAN_MINOR=6
OPENWSMAN_PATCH=1
OPENWSMAN_VERSION=${OPENWSMAN_MAJOR}.${OPENWSMAN_MINOR}.${OPENWSMAN_PATCH}

OPENWSMAN_PLUGIN_API_MAJOR=2
OPENWSMAN_PLUGIN_API_MINOR=2
OPENWSMAN_PLUGIN_API_VERSION=${OPENWSMAN_PLUGIN_API_MAJOR}.${OPENWSMAN_PLUGIN_API_MINOR}

AM_INIT_AUTOMAKE([subdir-objects])

AM_CONFIG_HEADER(wsman_config.h)
AM_MAINTAINER_MODE
AC_PROG_MAKE_SET
AC_PROG_CXX

WSMAN_PKG=wsman
WSMAN_CLIENT_PKG=${WSMAN_PKG}_client
WSMAN_CLIENT_TRANSPORT_PKG=${WSMAN_PKG}_curl_client_transport
WSMAN_CLIENTPP_PKG=${WSMAN_PKG}_clientpp
WSMAN_SERVER_PKG=${WSMAN_PKG}_server

AC_SUBST(WSMAN_UTILS_PKG)
AC_SUBST(WSMAN_CLIENT_PKG)
AC_SUBST(WSMAN_CLIENT_TRANSPORT_PKG)
AC_SUBST(WSMAN_CLIENTPP_PKG)
AC_SUBST(WSMAN_SERVER_PKG)
AC_SUBST(WSMAN_PKG)

case "$host" in
  *-*-solaris*)
	solaris="yes"
	;;
  *)
	;;
esac


dnl ***************************
dnl *** Set debugging flags ***
dnl ***************************

# Declare --enable-* args and collect ac_help strings
AC_ARG_ENABLE(debug, --enable-debug turn on debugging)

# Set the debug flags
if test "x$enable_debug" = "xyes"; then
    test "$cflags_set" = set || CFLAGS="$CFLAGS -g"
fi

# Set the debug flags
if test "x$enable_debug" = "xyes"; then
    AC_DEFINE(WSMAN_DEBUG_VERBOSE, 1, [Defined if verbose debug logging is requested])
fi


eventing_default=yes
AC_ARG_ENABLE(eventing,
		[  --enable-eventing=[no/yes] turn on WS-Eventing [default=yes]],,
              enable_eventing=$eventing_default)
if test "x$enable_eventing" = "xyes"; then
    AC_DEFINE(ENABLE_EVENTING_SUPPORT, 1, [Defined if WS-Eventing wanted])
fi
AM_CONDITIONAL(ENABLE_EVENTING_SUPPORT, test "x$enable_eventing" = "xyes")

dnl copied from openwbem :)
use_pam=0       
AC_ARG_ENABLE(pam,
AC_HELP_STRING(--disable-pam,disable pam integration))
if test "$enable_pam" != no; then
        use_pam=1
        AC_CHECK_HEADERS(pam/pam_misc.h pam/pam_appl.h, true, use_pam=0)
        if test x$use_pam = x0; then
                AC_CHECK_HEADERS(security/pam_misc.h security/pam_appl.h,
                        use_pam=1, use_pam=0)
        fi
                
        if test x$use_pam = x1; then
                AC_DEFINE([HAVE_PAM], [1], [Defined if pam support is available])
                PAM_LIB="-lpam"
        else
                AC_MSG_WARN(*** No PAM headers found! The PAM authentication module ***)
                AC_MSG_WARN(*** will not be built. The config file defaults to      ***)
                AC_MSG_WARN(*** using PAM authentication.                           ***)
                AC_MSG_WARN(*** You must edit the config file and use a different   ***)
                AC_MSG_WARN(*** authentication module or else the daemon won't start***)
        fi
fi                      
AC_SUBST(PAM_LIB)
AM_CONDITIONAL(USE_PAM, test x$use_pam = x1)


dnl ***************************
dnl *** Checks for programs ***
dnl ***************************
AC_PROG_CC
AM_PROG_CC_STDC
AC_PROG_INSTALL
# Initialize libtool
AM_PROG_LIBTOOL

# This isn't a program, but it doesn't fit anywhere else...
AC_FUNC_ALLOCA

AC_CHECK_FUNCS(strtok_r \
getpid \
strsep \
gettimeofday \
unlink \
syslog \
sleep \
fnmatch \
timegm \
daemon  \
va_copy \
memmove \
bcopy \
gmtime_r \
floor
)
AC_CHECK_FUNC(socket, , AC_CHECK_LIB(socket, socket))
AC_CHECK_FUNC(gethostbyname, , AC_CHECK_LIB(nsl, gethostbyname))
AC_CHECK_FUNCS(inet_pton inet_ntop inet_aton getaddrinfo getnameinfo)
#
# We do the IPv6 compilation checking after libtool so that we can put
# the right suffix on the files.
#
AC_MSG_CHECKING(for IPv6 structures)
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>],
[struct sockaddr_in6 sin6; return (0);],
	[AC_MSG_RESULT(yes)
	 found_ipv6=yes],
	[AC_MSG_RESULT(no)
	 found_ipv6=no])

if test "$soup_cv_ipv6" != "no"; then
    AC_DEFINE(HAVE_IPV6, 1, [Defined if you have IPV6 support])
    AC_SUBST(HAVE_IPV6)
fi

# check for ssize_t
AC_CHECK_TYPE(ssize_t, int)

# Set STDC_HEADERS
AC_HEADER_STDC
AC_CHECK_HEADERS([inttypes.h stdlib.h])
AC_CHECK_HEADERS([net/if.h net/if_dl.h])
AC_CHECK_HEADERS([sys/ioctl.h sys/sockio.h])
AC_CHECK_HEADERS([crypt.h sys/ioctl.h dirent.h])
AC_CHECK_HEADERS([vararg.h stdarg.h pthread.h])
AC_CHECK_HEADERS([unistd.h sys/types.h sys/sendfile.h sys/signal.h])
AC_CHECK_HEADERS([ctype.h sys/resource.h sys/socket.h sys/select.h])
AC_CHECK_HEADERS([netinet/in.h], [], [],
[#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
])

AC_HEADER_TIME
dnl Checks for types
AC_CHECK_TYPES([sa_family_t, in_port_t, in_addr_t], , ,
[
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#if HAVE_ARPA_NAMESER_H
# include <arpa/nameser.h>
#endif
])

AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
SIZEOF_SHORT=$ac_cv_sizeof_short
SIZEOF_INT=$ac_cv_sizeof_int
SIZEOF_LONG=$ac_cv_sizeof_long
SIZEOF_LONG_LONG=$ac_cv_sizeof_long_long
AC_SUBST(SIZEOF_SHORT)
AC_SUBST(SIZEOF_INT)
AC_SUBST(SIZEOF_LONG)
AC_SUBST(SIZEOF_LONG_LONG)
AC_CHECK_FUNCS(srandom)
AC_CHECK_MEMBER(struct sockaddr.sa_len,
       AC_DEFINE_UNQUOTED(HAVE_SA_LEN,
       1,
       [Define if struct sockaddr contains sa_len]),,
       [#include <sys/typesDD.h>
       #include <sys/socket.h>])

dnl *********************************
dnl *** Networking library checks ***
dnl *********************************

PKG_CHECK_MODULES(XML, libxml-2.0) AC_SUBST(XML_CFLAGS) AC_SUBST(XML_LIBS)
AC_PATH_PROG(CURL_CONFIG, curl-config, no)
if test "$CURL_CONFIG" = "no" ; then
	AC_MSG_ERROR([the curl-config script installed by libcurl could not be found.  libcurl is required for tests.])
fi
CURL_CFLAGS=`sh $CURL_CONFIG --cflags`
CURL_LIBS=`sh $CURL_CONFIG --libs`
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)

dnl Need libcurl version 7.12.0 or higher
CURLSTRVERSION="`sh $CURL_CONFIG --version`"
CURLNVERSION=`echo $CURLSTRVERSION | $SED -e 's/[[^0-9]]/ /g' | $AWK '{print ($1*10000) + ($2*100) + $3}'`
if test "$CURLNVERSION" -lt  71200 ; then
	AC_MSG_ERROR([
Openwsman requires libcurl version 7.12.00 or higher, and you 
are running "$CURLSTRVERSION".  You may be able to install a newer 
version via your operating system update facility, or you can 
retrieve it from:

    http://curl.haxx.se/

and install it directly.]
)
	exit
fi

dnl [ curl-Bugs-1924441 ] SSL callback option with NSS-linked libcurl
dnl CURLOPT_SSL_CTX_FUNCTION is defined, but does not work on e.g. Fedora

if test -f "/etc/fedora-release"; then
  CFLAGS="$CFLAGS -DNO_SSL_CALLBACK"
fi

#
# IPv6
#
AC_ARG_ENABLE(ipv6,
	[  --enable-ipv6		enable IPv6 support [default=disable]])

case "$enable_ipv6" in
	yes)
	        if test "$found_ipv6" = "no"; then
		  AC_MSG_WARN([No IPv6 support detected in system, disabling IPv6.])
		else
		  AC_DEFINE([ENABLE_IPV6],1,[Define if you want IPV6 support])
		  AC_SUBST(ENABLE_IPV6)
		fi
		;;
	no)
		;;
esac

examples_default=no
AC_ARG_WITH(examples,
		[  --with-examples=[no/yes] build examples [default=no]],,
              with_examples=$examples_default)
AM_CONDITIONAL(BUILD_EXAMPLES, test "x$with_examples" = "xyes")
tests_default=no
AC_ARG_WITH(tests,
		[  --with-tests=[no/yes] build tests [default=no]],,
              with_tests=$tests_default)
AM_CONDITIONAL(BUILD_TESTS, test "x$with_tests" = "xyes")

java_default=no
AC_ARG_ENABLE(java,
		[  --enable-java=[no/yes] build java bindings [default=no]],,
              enable_java=$java_default)

if test "x$enable_java" = "xyes"; then
AC_PROG_SWIG(1.3.30)
AC_CHECK_PROG(JAVA_COMPILER,javac,yes)
fi
AM_CONDITIONAL(BUILD_JAVA, test "x$enable_java" = "xyes")

python_default=no
AC_ARG_ENABLE(python,
		[  --enable-python=[no/yes] build python bindings [default=no]],,
              enable_python=$python_default)

if test "x$enable_python" = "xyes"; then
AM_PATH_PYTHON
AC_PROG_SWIG(1.3.30)
AC_CHECK_PROG(PYTHON_CONFIG,python-config,yes)
if test "$PYTHON_CONFIG" = "yes" ; then
	PYTHON_CFLAGS=`python-config --cflags`
	PYTHON_LIBS=`python-config --libs`
	AC_SUBST(PYTHON_CFLAGS)
	AC_SUBST(PYTHON_LIBS)
else
        PYTHON_VERSION=`python -c "import sys; print sys.version[[:3]]"`
        PYTHON_CFLAGS="-I/usr/include/python$PYTHON_VERSION"
	PYTHON_LIBS="-lpython$PYTHON_VERSION"
fi
fi
AM_CONDITIONAL(BUILD_PYTHON, test "x$enable_python" = "xyes")

ruby_default=no
AC_ARG_ENABLE(ruby,
                [  --enable-ruby=[no/yes] build Ruby bindings [default=no]],,
              enable_ruby=$ruby_default)

if test "x$enable_ruby" = "xyes"; then
  AC_PROG_SWIG(1.3.17)

  dnl Check for Ruby, taken from http://vim.svn.sourceforge.net
  AC_SUBST(vi_cv_path_ruby)
  AC_PATH_PROG(vi_cv_path_ruby, ruby)
  if test "X$vi_cv_path_ruby" != "X"; then
    AC_MSG_CHECKING(Ruby version)
    if `$vi_cv_path_ruby -e '(VERSION rescue RUBY_VERSION) >= "1.6.0" or exit 1'` >/dev/null 2>/dev/null; then
      AC_MSG_RESULT(OK)
      AC_MSG_CHECKING(Ruby header files)
      rubyhdrdir=`$vi_cv_path_ruby -r mkmf -e 'print Config::CONFIG[["archdir"]] || $hdrdir'` 2>/dev/null
      if test "X$rubyhdrdir" != "X"; then
        AC_MSG_RESULT($rubyhdrdir)
        RUBY_CFLAGS="$CFLAGS -I$rubyhdrdir"
        rubylibs=`$vi_cv_path_ruby -r rbconfig -e 'print Config::CONFIG[["LIBS"]]'`
	if test "X$rubylibs" != "X"; then
          RUBY_LIBS="$rubylibs"
        fi
        librubyarg=`$vi_cv_path_ruby -r rbconfig -e 'print Config.expand(Config::CONFIG[["LIBRUBYARG"]])'`
	if test -f "$rubyhdrdir/$librubyarg"; then
          librubyarg="$rubyhdrdir/$librubyarg"
        else
          rubylibdir=`$vi_cv_path_ruby -r rbconfig -e 'print Config.expand(Config::CONFIG[["libdir"]])'`
	  if test -f "$rubylibdir/$librubyarg"; then
            librubyarg="$rubylibdir/$librubyarg"
          elif test "$librubyarg" = "libruby.a"; then
            dnl required on Mac OS 10.3 where libruby.a doesn't exist
	    librubyarg="-lruby"
	  else
	    librubyarg=`$vi_cv_path_ruby -r rbconfig -e "print '$librubyarg'.gsub(/-L\./,%'-L#{Config.expand(Config::CONFIG[\"libdir\"])}')"`
	  fi
        fi

        if test "X$librubyarg" != "X"; then
          RUBY_LIBS="$librubyarg $RUBY_LIBS"
        fi
        rubyldflags=`$vi_cv_path_ruby -r rbconfig -e 'print Config::CONFIG[["LDFLAGS"]]'`
	if test "X$rubyldflags" != "X"; then
          RUBY_LDFLAGS="$rubyldflags $LDFLAGS"
        fi
      else
	AC_MSG_WARN(not found - disabling Ruby; Try installing ruby-devel)
      fi
    else
      AC_MSG_WARN(too old; need Ruby version 1.6.0 or later)
    fi
  fi
fi
AM_CONDITIONAL(BUILD_RUBY, test "x$enable_ruby" = "xyes")

java_default=no
AC_ARG_ENABLE(java,
                [  --enable-java=[no/yes] build Java bindings [default=no]],,
              enable_java=$java_default)
if test "x$enable_java" = "xyes"; then
  AC_PROG_SWIG(1.3.17)
fi
AM_CONDITIONAL(BUILD_JAVA, test "x$enable_java" = "xyes")

perl_default=no
AC_ARG_ENABLE(perl,
                [  --enable-perl=[no/yes] build Perl bindings [default=no]],,
              enable_perl=$perl_default)
AM_CONDITIONAL(BUILD_PERL, test "x$enable_perl" = "xyes")

if test "x$enable_perl" = "xyes"; then
  AC_PROG_SWIG(1.3.17)
  AC_MSG_CHECKING(Perl configuration)
  PERLCCFLAGS=`perl -MConfig -e 'print $Config{ccflags}'`
  PERLCOREDIR=`perl -MConfig -e 'print $Config{archlib}."/CORE"'`
  PERLVENDORARCHDIR=`perl -MConfig -e 'print $Config{installvendorarch}'`
  PERLVENDORLIBDIR=`perl -MConfig -e 'print $Config{installvendorlib}'`
  PERLLDFLAGS=`perl -MConfig -e 'print $Config{ccdlflags}'`
  AC_MSG_RESULT($PERLVENDORARCHDIR)
  AC_SUBST(PERLCCFLAGS)
  AC_SUBST(PERLCOREDIR)
  AC_SUBST(PERLVENDORARCHDIR)
  AC_SUBST(PERLVENDORLIBDIR)
  AC_SUBST(PERLLDFLAGS)
fi

AC_ARG_ENABLE(ssl, [  --enable-ssl             Turn on Secure Sockets Layer support [default=yes]],,
              enable_ssl=auto)

if test "$enable_ssl" != "no"; then
    PKG_CHECK_MODULES(OPENSSL, openssl , have_ssl="yes", have_ssl="no")
    AC_SUBST(OPENSSL_CFLAGS)
    AC_SUBST(OPENSSL_LIBS)
    if test "$have_ssl" != "yes"; then
        if test "$enable_ssl" = "auto"; then
            AC_MSG_WARN(Disabling SSL support);
            enable_ssl=no;
        else
            AC_MSG_ERROR(Could not configure SSL support);
        fi
    fi
fi

if test "$enable_ssl" != "no"; then
    AC_DEFINE(HAVE_SSL, 1, [Defined if you have SSL support])
fi
AM_CONDITIONAL(USE_OPENSSL, test "x$enable_ssl" != "xno")


AH_TEMPLATE(HAVE_LIBCRYPT, [libcrypt library present])
AC_CHECK_FUNCS([crypt], HAVE_LIBC_CRYPT="true")
if test -z "$HAVE_LIBC_CRYPT"; then
       AC_CHECK_LIB(crypt, crypt,
             CRYPT_LIBS="-lcrypt"
             [AC_DEFINE(HAVE_LIBCRYPT)], ,)
fi

AC_SUBST(CRYPT_LIBS)

dnl
dnl Use built-in UUID generation if on Solaris
dnl

if test x"$solaris" = x"yes" ; then
	LDFLAGS="$LDFLAGS -luuid"
fi

if test "$prefix" = "NONE"; then
    prefix=$ac_default_prefix;
fi
SYSCONFDIR_TMP="`eval echo $sysconfdir`"
AC_DEFINE_UNQUOTED([DEFAULT_CONFIG_FILE], ["${SYSCONFDIR_TMP}/openwsman/openwsman.conf"] , [Default config file])
AC_DEFINE_UNQUOTED([DEFAULT_CLIENT_CONFIG_FILE], ["${SYSCONFDIR_TMP}/openwsman/openwsman_client.conf"] , [Default client config file])
SYSCONFDIR=${sysconfdir}/openwsman

test x"$localstatedir" = x"" && localstatedir=/var
dnl Subst PACKAGE_PLUGIN_DIR.
packagesubscriptiondir="${localstatedir}"/lib/${PACKAGE}/subscriptions
PACKAGE_SUBSCRIPTION_DIR="${packagesubscriptiondir}"
AC_SUBST(PACKAGE_SUBSCRIPTION_DIR)

dnl Subst PACKAGE_PLUGIN_DIR.
packageplugindir=$libdir/${PACKAGE}/plugins
PACKAGE_PLUGIN_DIR="${packageplugindir}"
AC_SUBST(PACKAGE_PLUGIN_DIR)

dnl Subst PACKAGE_AUTH_DIR.
packageauthdir=$libdir/${PACKAGE}/authenticators
PACKAGE_AUTH_DIR="${packageauthdir}"
AC_SUBST(PACKAGE_AUTH_DIR)

AC_SUBST(PACKAGE)
AC_SUBST(OPENWSMAN_VERSION)
AC_SUBST(OPENWSMAN_MAJOR)
AC_SUBST(OPENWSMAN_MINOR)
AC_SUBST(OPENWSMAN_PATCH)

AC_SUBST(OPENWSMAN_PLUGIN_API_MINOR)
AC_SUBST(OPENWSMAN_PLUGIN_API_MAJOR)
AC_SUBST(OPENWSMAN_PLUGIN_API_VERSION)

AC_DEFINE_UNQUOTED(OPENWSMAN_PLUGIN_API_VERSION,
          ["$OPENWSMAN_PLUGIN_API_VERSION"],
          [Plugin API version])

AC_CHECK_LIB([cmpisfcc],  [cmciConnect], [have_sfcc="yes"],[have_sfcc="no"])
AM_CONDITIONAL(BUILD_LIBCIM, test "$have_sfcc" = "yes" )

have_cunit=no
AC_CHECK_HEADERS([CUnit/Basic.h], have_cunit="yes" )

AM_CONDITIONAL(BUILD_CUNIT_TESTS, test "$have_cunit" = "yes")

AC_ARG_ENABLE(server,
AC_HELP_STRING(--disable-server,disable server))

OWMAN_DISABLE_SERVER=0
if test "$enable_server" = no; then
        OWMAN_DISABLE_SERVER=1
fi
AM_CONDITIONAL(DISABLE_SERVER, test x$OWMAN_DISABLE_SERVER = x1)

AC_ARG_ENABLE(plugins,
AC_HELP_STRING(--disable-plugins,disable plugins))

OWMAN_DISABLE_PLUGINS=0
if test "$enable_plugins" = no; then
        OWMAN_DISABLE_PLUGINS=1
fi
AM_CONDITIONAL(DISABLE_PLUGINS, test x$OWMAN_DISABLE_PLUGINS = x1)

AC_ARG_ENABLE(more-warnings,
	      [  --disable-more-warnings  Inhibit compiler warnings],
	      set_more_warnings=no)

if test "$GCC" = "yes" -a "$set_more_warnings" != "no"; then
	CFLAGS="$CFLAGS \
		-Wall -Wstrict-prototypes -Wmissing-declarations \
		-Wmissing-prototypes -Wnested-externs -Wpointer-arith \
                -Wunused -Werror"
fi



dnl *************************
dnl *** Output Everything ***
dnl *************************
AC_SUBST(SYSCONFDIR)

AC_CONFIG_FILES([
        openwsman.pc
        openwsman++.pc
        openwsman-server.pc
        package/openwsman.spec])

AC_OUTPUT([
        Doxyfile
        Makefile
        src/Makefile
        src/server/Makefile
        src/lib/Makefile
        src/plugins/Makefile
        src/plugins/cim/Makefile
        src/plugins/identify/Makefile
        src/plugins/wsman/Makefile
        src/plugins/wsman/test/Makefile
        src/authenticators/Makefile
        src/authenticators/file/Makefile
        src/authenticators/pam/Makefile
        src/cpp/Makefile
        include/Makefile
        include/cim/Makefile
        include/u/Makefile
        doc/Makefile
        doc/specs/Makefile
        etc/Makefile
        etc/owsmangencert.sh
        etc/init/Makefile
        etc/init/openwsmand.sh
        tests/Makefile
        tests/client/Makefile
	tests/epr/Makefile
	tests/filter/Makefile
        tests/xml/Makefile
        examples/Makefile
	bindings/Makefile
	bindings/version.i
	bindings/python/Makefile
	bindings/python/tests/Makefile
	bindings/ruby/Makefile
	bindings/ruby/tests/Makefile
	bindings/ruby/openwsman/Makefile
	bindings/java/Makefile
	bindings/java/tests/Makefile
	bindings/perl/Makefile
	bindings/perl/tests/Makefile
	])

if test "x$enable_ruby" = "xyes" -o "x$enable_python" = "xyes" -o "x$enable_perl" = "xyes"; then
  if test "$set_more_warnings" != "no"; then
    AC_MSG_WARN(*** Consider setting --disable-more-warnings else bindings will likely fail to build ***)
  fi
fi

AC_OUTPUT

echo =================================================================
echo -e "Configuration Features:\n"
echo -e "build plugins"\\t\\t\\t"${enable_plugins:-yes}"
echo -e "build server"\\t\\t\\t"${enable_server:-yes}"
echo -e "debug"\\t\\t\\t\\t"${enable_debug:-no}"
echo -e "eventing"\\t\\t\\t"${enable_eventing:-yes}"
echo -e "examples"\\t\\t\\t"${with_examples:-no}"
echo -e "IPv6"\\t\\t\\t\\t"${enable_ipv6:-no}"
echo -e "java binding"\\t\\t\\t"${enable_java:-no}"
echo -e "PAM authentication"\\t\\t"${enable_pam:-yes}"
echo -e "python binding"\\t\\t\\t"${enable_python:-no}"
echo -e "perl binding"\\t\\t\\t"${enable_perl:-no}"
echo -e "ruby binding"\\t\\t\\t"${enable_ruby:-no}"
echo -e "ssl"\\t\\t\\t\\t"${enable_ssl:-yes}"
echo -e "tests"\\t\\t\\t\\t"${with_tests:-no}"
echo =================================================================
echo
echo "openwsman configuration complete.  you may now run 'make'"
echo

