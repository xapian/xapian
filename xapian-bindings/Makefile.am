## Process this file with automake to produce Makefile.in

# `make QUIET=' overrides `./configure --enable-quiet'.
# `make QUIET=y' overrides `./configure' without `--enable-quiet'.
LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped SVN tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = xapian.i xapian-head.i generic/except.i \
    xapian-bindings.spec.in xapian-bindings.spec skiptest \
    xapian-version.h.in xapian-version.h HACKING TODO
DISTCLEANFILES = xapian-version.h

SUBDIRS = @BINDINGS@
DIST_SUBDIRS = python php java-swig java ruby tcl8 csharp # guile

DISTCHECK_CONFIGURE_FLAGS = XAPIAN_CONFIG="$(XAPIAN_CONFIG)" \
	PYTHON_LIB="`pwd`/../_inst/tmp_pylib" \
	PHP_EXTENSION_DIR="`pwd`/../_inst/tmp_phpext" \
	RUBY_LIB="`pwd`/../_inst/tmp_rubylib" \
	RUBY_LIB_ARCH="`pwd`/../_inst/tmp_rubylibarch" \
	TCL_LIB="`pwd`/../_inst/tmp_tcllib"

# FIXME: this is wrong. Should be in EXTRA_DIST? Hmm.
# See the */docs/Makefile.am and do something similar.
if !MAINTAINER_NO_DOCS
toolsdir=$(srcdir)/../xapian-maintainer-tools/html-tools
docdatadir=$(docdir)
dist_docdata_DATA = $(toolsdir)/style.css \
	$(toolsdir)/xapian-logo.png \
	$(toolsdir)/graded-background.png
endif
