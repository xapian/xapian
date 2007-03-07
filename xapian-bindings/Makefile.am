## Process this file with automake to produce Makefile.in

EXTRA_DIST = swig/xapian.i swig/defaults/except.i \
    xapian-bindings.spec.in xapian-bindings.spec skiptest \
    xapian-version.h.in xapian-version.h HACKING TODO
DISTCLEANFILES = xapian-version.h

SUBDIRS = @BINDINGS@
DIST_SUBDIRS = python php java ruby tcl8 csharp # guile
