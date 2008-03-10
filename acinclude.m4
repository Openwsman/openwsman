dnl ------------------------------------------------------------------------
dnl Find a file (or one of more files in a list of dirs)
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN([AC_FIND_FILE],
[
$3=NO
for i in $2;
do
  for j in $1;
  do
    echo "configure: __oline__: $i/$j" >&AC_FD_CC
    if test -r "$i/$j"; then
      echo "taking that" >&AC_FD_CC
      $3=$i
      break 2
    fi
  done
done
])

AC_DEFUN([CHECK_SFCC],
[

ac_SFCC_includes=NO 
ac_SFCC_libraries=NO
SFCC_libraries=""
SFCC_includes=""
sfcc_error=""
AC_ARG_WITH(sfcc-dir,
        [  --with-sfcc-dir=DIR      where the root of sfcc is installed],
        [  ac_SFCC_includes="$withval"/include
                ac_SFCC_libraries="$withval"/lib${libsuff}
        ])

want_SFCC=yes
if test $want_SFCC = yes; then
        AC_MSG_CHECKING(for Sfcc)

        AC_CACHE_VAL(ac_cv_have_SFCC,
        [#try to guess Sfcc locations

                SFCC_incdirs="/usr/include /usr/local/include /usr/sfcc/include /usr/local/sfcc/include $prefix/include"
                SFCC_incdirs="$ac_SFCC_includes $SFCC_incdirs"
                AC_FIND_FILE([CimClientLib/cimcClient.h], $SFCC_incdirs, SFCC_incdir)
                ac_SFCC_includes="$SFCC_incdir"

                SFCC_libdirs="/usr/lib${libsuff} /usr/local/lib${libsuff} /usr/sfcc/lib /usr/local/sfcc/lib $prefix/lib${libsuff} $exec_prefix/lib${libsuff}"
                if test ! "$ac_SFCC_libraries" = "NO"; then
                        SFCC_libdirs="$ac_SFCC_libraries $SFCC_libdirs"
                fi
                test=NO
                SFCC_libdir=NO
                for dir in $SFCC_libdirs; do
                        try="ls -1 $dir/libcimc*"
                        if test=`eval $try 2> /dev/null`; then SFCC_libdir=$dir; break; else echo "tried $dir" >&AC_FD_CC ; fi
                done

                ac_SFCC_libraries="$SFCC_libdir"

                if test "$ac_SFCC_includes" = NO || test "$ac_SFCC_libraries" = NO; then
                        have_SFCC=no
                        sfcc_error="Sfcc not found"
                else
                        have_SFCC=yes;
                fi

        ])

        eval "$ac_cv_have_SFCC"

        AC_MSG_RESULT([libraries $ac_SFCC_libraries, headers $ac_SFCC_includes])

else
        have_SFCC=no
        sfcc_error="Sfcc not found"
fi

if test "$ac_SFCC_includes" = "/usr/include" || test  "$ac_SFCC_includes" = "/usr/local/include" || test -z "$ac_SFCC_includes"; then
        SFCC_INCLUDES="";
else
        SFCC_INCLUDES="-I$ac_SFCC_includes"
fi

if test "$ac_SFCC_libraries" = "/usr/lib" || test "$ac_SFCC_libraries" = "/usr/local/lib" || test -z "$ac_SFCC_libraries"; then
        SFCC_LDFLAGS=""
else
        SFCC_LDFLAGS="-L$ac_SFCC_libraries -R$ac_SFCC_libraries"
fi

AC_SUBST(SFCC_INCLUDES)
AC_SUBST(SFCC_LDFLAGS)
])
##### http://autoconf-archive.cryp.to/ac_pkg_swig.html
#
# SYNOPSIS
#
#   AC_PROG_SWIG([major.minor.micro])
#
# DESCRIPTION
#
#   This macro searches for a SWIG installation on your system. If
#   found you should call SWIG via $(SWIG). You can use the optional
#   first argument to check if the version of the available SWIG is
#   greater than or equal to the value of the argument. It should have
#   the format: N[.N[.N]] (N is a number between 0 and 999. Only the
#   first N is mandatory.)
#
#   If the version argument is given (e.g. 1.3.17), AC_PROG_SWIG checks
#   that the swig package is this version number or higher.
#
#   In configure.in, use as:
#
#     AC_PROG_SWIG(1.3.17)
#     SWIG_ENABLE_CXX
#     SWIG_MULTI_MODULE_SUPPORT
#     SWIG_PYTHON
#
# LAST MODIFICATION
#
#   2006-10-22
#
# COPYLEFT
#
#   Copyright (c) 2006 Sebastian Huber <sebastian-huber@web.de>
#   Copyright (c) 2006 Alan W. Irwin <irwin@beluga.phys.uvic.ca>
#   Copyright (c) 2006 Rafael Laboissiere <rafael@laboissiere.net>
#   Copyright (c) 2006 Andrew Collier <colliera@ukzn.ac.za>
#
#   This program is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation; either version 2 of the
#   License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#   02111-1307, USA.
#
#   As a special exception, the respective Autoconf Macro's copyright
#   owner gives unlimited permission to copy, distribute and modify the
#   configure scripts that are the output of Autoconf when processing
#   the Macro. You need not follow the terms of the GNU General Public
#   License when using or distributing such scripts, even though
#   portions of the text of the Macro appear in them. The GNU General
#   Public License (GPL) does govern all other use of the material that
#   constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the
#   Autoconf Macro released by the Autoconf Macro Archive. When you
#   make and distribute a modified version of the Autoconf Macro, you
#   may extend this special exception to the GPL to apply to your
#   modified version as well.

AC_DEFUN([AC_PROG_SWIG],[
        AC_PATH_PROG([SWIG],[swig])
        if test -z "$SWIG" ; then
                AC_MSG_WARN([cannot find 'swig' program. You should look at http://www.swig.org])
                SWIG='echo "Error: SWIG is not installed. You should look at http://www.swig.org" ; false'
        elif test -n "$1" ; then
                AC_MSG_CHECKING([for SWIG version])
                [swig_version=`$SWIG -version 2>&1 | grep 'SWIG Version' | sed 's/.*\([0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\).*/\1/g'`]
                AC_MSG_RESULT([$swig_version])
                if test -n "$swig_version" ; then
                        # Calculate the required version number components
                        [required=$1]
                        [required_major=`echo $required | sed 's/[^0-9].*//'`]
                        if test -z "$required_major" ; then
                                [required_major=0]
                        fi
                        [required=`echo $required | sed 's/[0-9]*[^0-9]//'`]
                        [required_minor=`echo $required | sed 's/[^0-9].*//'`]
                        if test -z "$required_minor" ; then
                                [required_minor=0]
                        fi
                        [required=`echo $required | sed 's/[0-9]*[^0-9]//'`]
                        [required_patch=`echo $required | sed 's/[^0-9].*//'`]
                        if test -z "$required_patch" ; then
                                [required_patch=0]
                        fi
                        # Calculate the available version number components
                        [available=$swig_version]
                        [available_major=`echo $available | sed 's/[^0-9].*//'`]
                        if test -z "$available_major" ; then
                                [available_major=0]
                        fi
                        [available=`echo $available | sed 's/[0-9]*[^0-9]//'`]
                        [available_minor=`echo $available | sed 's/[^0-9].*//'`]
                        if test -z "$available_minor" ; then
                                [available_minor=0]
                        fi
                        [available=`echo $available | sed 's/[0-9]*[^0-9]//'`]
                        [available_patch=`echo $available | sed 's/[^0-9].*//'`]
                        if test -z "$available_patch" ; then
                                [available_patch=0]
                        fi
                        if test $available_major -ne $required_major \
                                -o $available_minor -ne $required_minor \
                                -o $available_patch -lt $required_patch ; then
                                AC_MSG_WARN([SWIG version >= $1 is required.  You have $swig_version.  You should look at http://www.swig.org])
                                SWIG='echo "Error: SWIG version >= $1 is required.  You have '"$swig_version"'.  You should look at http://www.swig.org" ; false'
                        else
                                AC_MSG_NOTICE([SWIG executable is '$SWIG'])
                                SWIG_LIB=`$SWIG -swiglib`
                                AC_MSG_NOTICE([SWIG library directory is '$SWIG_LIB'])
				SWIG_VERSION=`echo $(( $available_major * 100 * 100 + $available_minor * 100 + $available_patch ))`
                                AC_MSG_NOTICE([SWIG version is '$SWIG_VERSION'])
				# AM_CONDITIONAL(SWIG_NEW_OPTIONS, test "$SWIG_VERSION" \> 10331)
                        fi
                else
                        AC_MSG_WARN([cannot determine SWIG version])
                        SWIG='echo "Error: Cannot determine SWIG version.  You should look at http://www.swig.org" ; false'
                fi
        fi
        AC_SUBST([SWIG_LIB])
])
