#!/bin/sh

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

"$srcdir/preautoreconf"
autoreconf -fiv
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@"
