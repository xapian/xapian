## Process this file with automake to produce Makefile.in

EXTRA_DIST = util.i extra.i
DIST_SUBDIRS = docs
SUBDIRS = docs

pylibdir = @PYTHON_LIB@/site-packages

pylib_SCRIPTS = xapian.py

pylib_LTLIBRARIES = _xapian.la

# Remove the .la file - _xapian.la is never linked against (it's a module)
# and Python doesn't use libltdl.
# FIXME: -hook seems to execute first, not after everything else as the
# automake manual says it does, so (ab)use install-exec-local instead.
# Investigate and report to libtool developers...
install-exec-local:
	rm -f $(DESTDIR)$(pylibdir)/_xapian.la

AM_CPPFLAGS = -I@PYTHON_INC@
# swig generated code gives lots of (non-harmful) warnings with -W or -Wunused
AM_CXXFLAGS = $(XAPIAN_CXXFLAGS) -Wall -Wno-unused
_xapian_la_LDFLAGS = -avoid-version -module
nodist__xapian_la_SOURCES = xapian_wrap.cc
_xapian_la_LIBADD = $(XAPIAN_LIBS)

BUILT_SOURCES = xapian_wrap.cc xapian.py

xapian_wrap.cc xapian.py: ../xapian.i util.i extra.i
	$(SWIG) -I$(srcdir) $(XAPIAN_CXXFLAGS) -c++ -python -shadow -o $@ $(srcdir)/../xapian.i

CLEANFILES = $(BUILT_SOURCES)
