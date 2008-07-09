## Process this file with automake to produce Makefile.in

# `make QUIET=' overrides `./configure --enable-quiet'.
# `make QUIET=y' overrides `./configure' without `--enable-quiet'.
LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = xapian.i xapian-head.i generic/except.i \
    xapian-bindings.spec.in xapian-bindings.spec skiptest \
    xapian-version.h.in xapian-version.h HACKING TODO
DISTCLEANFILES = xapian-version.h

SUBDIRS = @BINDINGS@
DIST_SUBDIRS = python php java-swig java ruby tcl8 csharp # guile

# Ensure that the m4 directory is present, to avoid autoreconf failing.
dist-hook:
	-test -d $(distdir)/m4 || mkdir $(distdir)/m4

DISTCHECK_CONFIGURE_FLAGS = XAPIAN_CONFIG="$(XAPIAN_CONFIG)" \
	PYTHON_LIB="`pwd`/../_inst/tmp_pylib" \
	PHP_EXTENSION_DIR="`pwd`/../_inst/tmp_phpext" \
	RUBY_LIB="`pwd`/../_inst/tmp_rubylib" \
	RUBY_LIB_ARCH="`pwd`/../_inst/tmp_rubylibarch" \
	TCL_LIB="`pwd`/../_inst/tmp_tcllib"
