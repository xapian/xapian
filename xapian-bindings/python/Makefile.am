## Process this file with automake to produce Makefile.in

TESTS_ENVIRONMENT = PYTHONPATH=@PYTHON_MODERN_OR_OLDE@:.libs $(PYTHON)

## Test programs to be run
TESTS = smoketest.py

# Interleave modern and olde as the .cc-s and .h-s are probably identical,
# the .py-s are very similar, and gzip will compress this redundancy away if
# its close enough together for it to notice.
BUILT_SOURCES = \
	modern/xapian_wrap.cc olde/xapian_wrap.cc \
	modern/xapian_wrap.h olde/xapian_wrap.h \
	modern/xapian.py olde/xapian.py

EXTRA_DIST = util.i extra.i $(TESTS) $(BUILT_SOURCES)

SUBDIRS = docs

pylibdir = @PYTHON_LIB@/site-packages

pylib_SCRIPTS = @PYTHON_MODERN_OR_OLDE@/xapian.py xapian.pyc

pylib_LTLIBRARIES = _xapian.la

# Remove the .la file - _xapian.la is never linked against (it's a module)
# and Python doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(pylibdir)/_xapian.la

AM_CPPFLAGS = -I@PYTHON_INC@
AM_CXXFLAGS = @SWIG_CXXFLAGS@
_xapian_la_LDFLAGS = -avoid-version -module
_xapian_la_SOURCES = @PYTHON_MODERN_OR_OLDE@/xapian_wrap.cc \
	@PYTHON_MODERN_OR_OLDE@/xapian_wrap.h
_xapian_la_LIBADD = $(XAPIAN_LIBS)

xapian.pyc: @PYTHON_MODERN_OR_OLDE@/xapian.py
	PYTHONPATH=@PYTHON_MODERN_OR_OLDE@:.libs $(PYTHON) -c "import xapian"

if MAINTAINER_MODE
# "modern" versions are for Python 2.2 and up;  "olde" versions are for earlier
# Python versions (which in our case means 2.1).

modern/xapian_wrap.cc modern/xapian_wrap.h modern/xapian.py: ../xapian.i util.i extra.i
	-mkdir modern
	$(SWIG) -I$(srcdir) @SWIG_FLAGS@ -c++ -python -shadow -modern \
	    -o modern/xapian_wrap.cc $(srcdir)/../xapian.i

olde/xapian_wrap.cc olde/xapian_wrap.h olde/xapian.py: ../xapian.i util.i extra.i
	-mkdir olde
	$(SWIG) -I$(srcdir) @SWIG_FLAGS@ -c++ -python -shadow \
	    -o olde/xapian_wrap.cc $(srcdir)/../xapian.i

CLEANFILES = $(BUILT_SOURCES) xapian.pyc
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
CLEANFILES = xapian.pyc
endif
