## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include -I$(top_srcdir)/extra
SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig

pylibdir = @PYTHON_LIB@/site-packages

pylib_SCRIPTS = xapian.py

pylib_LTLIBRARIES = _xapian.la

# Remove the .la file - _xapian.la is never linked against (it's a module)
# and Python doesn't use libltdl.
# hmm -hook doesn't seem to execute after everything else
install-exec-local:
	rm -f $(DESTDIR)$(pylibdir)/_xapian.la

AM_CPPFLAGS = -I@PYTHON_INC@
# swig generated code gives lots of (non-harmful) warnings with -W or -Wunused
AM_CXXFLAGS = -Wall -Wno-unused
_xapian_la_LDFLAGS = -avoid-version -module
nodist__xapian_la_SOURCES = xapian_wrap.cc
_xapian_la_LIBADD = ../../libxapian.la ../../extra/libomqueryparser.la

BUILT_SOURCES = xapian_wrap.cc xapian.py

xapian_wrap.cc xapian.py: ../swig/xapian.i ../swig/omstem.i ../swig/om_util.i ../swig/om_util_python.i
	$(SWIG) $(SWIG_INCLUDES) -c++ -python -shadow -o $@ $(srcdir)/../swig/xapian.i

CLEANFILES = $(BUILT_SOURCES)
