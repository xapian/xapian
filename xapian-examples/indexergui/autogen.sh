#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="the package."

(test -f $srcdir/configure.in) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level directory"
    exit 1
}

. $srcdir/macros/autogen.sh
