## Process this file with automake to produce Makefile.in

TESTS_ENVIRONMENT = PYTHONPATH=.:.libs $(PYTHON)

## Test programs to be run
TESTS = smoketest.py

EXTRA_DIST = util.i extra.i $(TESTS)
DIST_SUBDIRS = docs
SUBDIRS = docs

pylibdir = @PYTHON_LIB@/site-packages

pylib_SCRIPTS = xapian.py xapian.pyc

pylib_LTLIBRARIES = _xapian.la

# Remove the .la file - _xapian.la is never linked against (it's a module)
# and Python doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(pylibdir)/_xapian.la

AM_CPPFLAGS = -I@PYTHON_INC@
AM_CXXFLAGS = $(XAPIAN_CXXFLAGS) @SWIG_CXXFLAGS@
_xapian_la_LDFLAGS = -avoid-version -module
nodist__xapian_la_SOURCES = xapian_wrap.cc
_xapian_la_LIBADD = $(XAPIAN_LIBS)

BUILT_SOURCES = xapian_wrap.cc xapian_wrap.h

xapian_wrap.cc xapian.py: ../xapian.i util.i extra.i
	$(SWIG) -I/usr/include -I$(srcdir) $(XAPIAN_CXXFLAGS) -c++ -python -shadow @SWIG_PYTHON_FLAGS@ -o xapian_wrap.cc $(srcdir)/../xapian.i

xapian.pyc: xapian.py
	PYTHONPATH=.:.libs $(PYTHON) -c "import xapian"

CLEANFILES = $(BUILT_SOURCES) xapian.py xapian.pyc
