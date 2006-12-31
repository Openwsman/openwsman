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
                test=NONE
                SFCC_libdir=NONE
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
