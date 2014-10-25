## Process this file with automake to produce Makefile.in

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped git tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = fake_dbfactory.i xapian-head.i xapian-headers.i generic/except.i \
    generic/generate-generic-exceptions \
    xapian-bindings.spec shortstop.list skiptest \
    HACKING

SUBDIRS = . @BINDINGS@
DIST_SUBDIRS = python python3 php java ruby tcl8 csharp perl lua

if MAINTAINER_MODE
BUILT_SOURCES = generic/except.i
generic/except.i: $(srcdir)/generic/generate-generic-exceptions ../xapian-core/exception_data.pm
	$(PERL) -w -I$(srcdir)/../xapian-core $(srcdir)/generic/generate-generic-exceptions

if !MAINTAINER_NO_DOCS
BUILT_SOURCES += doxygen-xml/index.xml

doxygen-xml/index.xml: $(XAPIAN_HEADERS)
	$(DOXYGEN) doxygen_xml.conf
endif
endif

DISTCHECK_CONFIGURE_FLAGS = XAPIAN_CONFIG="$(XAPIAN_CONFIG)" \
	PYTHON2_LIB="`pwd`/../_inst/tmp_py2lib" \
	PYTHON3_LIB="`pwd`/../_inst/tmp_py3lib" \
	PHP_EXTENSION_DIR="`pwd`/../_inst/tmp_phpext" \
	RUBY_LIB="`pwd`/../_inst/tmp_rubylib" \
	RUBY_LIB_ARCH="`pwd`/../_inst/tmp_rubylibarch" \
	TCL_LIB="`pwd`/../_inst/tmp_tcllib" \
	PERL_LIB="`pwd`/../_inst/tmp_perllib" \
	LUA_LIB="`pwd`/../_inst/tmp_lualib"
