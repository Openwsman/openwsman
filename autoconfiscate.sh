#!/bin/sh
aclocal &&
autoheader &&
libtoolize --force && 
automake -af &&
autoconf
