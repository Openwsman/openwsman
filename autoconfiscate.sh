#!/bin/sh -x

libtoolize --copy --force --automake
aclocal
autoheader
automake --add-missing --copy --foreign
autoconf
