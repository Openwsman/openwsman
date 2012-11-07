#!/bin/sh 

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
