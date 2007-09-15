## Process this file with automake to produce Makefile.in

EXTRA_DIST = xapian.i generic/except.i \
    xapian-bindings.spec.in xapian-bindings.spec skiptest \
    xapian-version.h.in xapian-version.h HACKING TODO
DISTCLEANFILES = xapian-version.h

SUBDIRS = @BINDINGS@
DIST_SUBDIRS = python php java-swig java ruby tcl8 csharp # guile

DISTCHECK_CONFIGURE_FLAGS = "XAPIAN_CONFIG=$(XAPIAN_CONFIG)" $(EXTRADISTCHECK_CONFIGURE_FLAGS)
