## Process this file with automake to produce Makefile.in

# `make QUIET=' overrides `./configure --enable-quiet'.
# `make QUIET=y' overrides `./configure' without `--enable-quiet'.
LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped git tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = xapian.i xapian-head.i generic/except.i \
    xapian-bindings.spec skiptest \
    HACKING

SUBDIRS = @BINDINGS@
DIST_SUBDIRS = python php java-swig java ruby tcl8 csharp perl lua

DISTCHECK_CONFIGURE_FLAGS = XAPIAN_CONFIG="$(XAPIAN_CONFIG)" \
	PYTHON_LIB="`pwd`/../_inst/tmp_pylib" \
	PHP_EXTENSION_DIR="`pwd`/../_inst/tmp_phpext" \
	RUBY_LIB="`pwd`/../_inst/tmp_rubylib" \
	RUBY_LIB_ARCH="`pwd`/../_inst/tmp_rubylibarch" \
	TCL_LIB="`pwd`/../_inst/tmp_tcllib" \
	PERL_LIB="`pwd`/../_inst/tmp_perllib" \
	LUA_LIB="`pwd`/../_inst/tmp_lualib"
