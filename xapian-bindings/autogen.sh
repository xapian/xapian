#!/bin/sh

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

autoreconf -fiv
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@"
