#!/bin/sh

cat <<EOS >&2
*** Autoconf/automake is deprecated for Openwsman and might not fully work.
*** Please use CMake instead!
*** Pull requests welcome ;-)
EOS

if [ "$1" != "--ignore-deprecation-warning" ]; then
  cat <<EOS >&2
*** To ignore this warning and proceed regardless, re-run as follows:
***   $0 --ignore-deprecation-warning
EOS
  exit 1
fi

UNAME=`uname`

mkdir -p m4

# Ouch, automake require this
cp README.md README

if [ "$UNAME" = "Darwin" ]; then
    LIBTOOLIZE=glibtoolize
else
    LIBTOOLIZE=libtoolize
fi

$LIBTOOLIZE --copy --force --automake
aclocal
autoheader
automake --add-missing --copy --foreign
autoconf
