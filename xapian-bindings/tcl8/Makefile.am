## Process this file with automake to produce Makefile.in

EXTRA_DIST = util.i extra.i

tcllibdir = @TCL_LIB@

tcllib_LTLIBRARIES = xapian.la

# Remove the .la file - xapian.la is never linked against (it's a module)
# and TCL doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(tcllibdir)/xapian.la

AM_CPPFLAGS = @TCL_CPPFLAGS@
AM_CXXFLAGS = $(XAPIAN_CXXFLAGS) @SWIG_CXXFLAGS@
xapian_la_LDFLAGS = -avoid-version -module
nodist_xapian_la_SOURCES = xapian_wrap.cc
xapian_la_LIBADD = $(XAPIAN_LIBS)

BUILT_SOURCES = xapian_wrap.cc

xapian_wrap.cc: ../xapian.i util.i extra.i
	$(SWIG) -I/usr/include -I$(srcdir) $(XAPIAN_CXXFLAGS) -c++ -tcl8 -o $@ $(srcdir)/../xapian.i

CLEANFILES = $(BUILT_SOURCES)
