## Process this file with automake to produce Makefile.in

EXTRA_DIST = util.i extra.i

tcllibdir = @TCL_LIB@

tcllib_LTLIBRARIES = xapian.la

AM_CPPFLAGS = -I@TCL_INC@
# swig generated code gives lots of (non-harmful) warnings with -W or -Wunused
AM_CXXFLAGS = $(XAPIAN_CXXFLAGS) -Wall -Wno-unused
xapian_la_LDFLAGS = -avoid-version -module
nodist_xapian_la_SOURCES = xapian_wrap.cc
xapian_la_LIBADD = $(XAPIAN_LIBS)

BUILT_SOURCES = xapian_wrap.cc

xapian_wrap.cc: ../xapian.i util.i extra.i
	$(SWIG) -I/usr/include -I$(srcdir) $(XAPIAN_CXXFLAGS) -c++ -tcl8 -o $@ $(srcdir)/../xapian.i

CLEANFILES = $(BUILT_SOURCES)
